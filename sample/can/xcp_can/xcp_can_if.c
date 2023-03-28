#include "XcpBasic.h"
#include "can/can.h"
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Types.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/BIOS.h>
#include "f2833x/v142/DSP2833x_headers/include/DSP2833x_Device.h"
#if defined(XCP_ENABLE_TESTMODE)
#include "uart/uart.h"
#endif

#define XCP_EVENT_PROCESS               (Event_Id_00)
#define XCP_EVENT_TX_DONE               (Event_Id_01)
#define XCP_EVENT_RX_PENDING            (Event_Id_02)

static Bool bInitDone = FALSE;
static Task_Handle xcpTaskHandle = NULL;
static Event_Handle xcp_event = NULL;
static Clock_Handle xcpClkHandle = NULL;
static Mailbox_Handle mbxCanRx = NULL;
static Mailbox_Handle mbxCanTx = NULL;

static void XcpTaskFxn(UArg a0, UArg a1);
static void XcpClkFxn(UArg a);

#if defined (XCP_ENABLE_DAQ_TIMESTAMP)
void ApplXcpTimestampInit(void);
#endif

void XcpCanInit(void)
{
    Error_Block eb;
    Mailbox_Params mbxParam;
    Task_Params tskParam;
    Clock_Params clkParam;
    Bool retBool = FALSE;

    if(!bInitDone) {
        CAN_init();

    #if defined(XCP_ENABLE_TESTMODE)
        UART_init();
    #endif

    #if defined (XCP_ENABLE_DAQ_TIMESTAMP)
        ApplXcpTimestampInit();
    #endif

        Error_init(&eb);
        /*
         * Event
         */
        xcp_event = Event_create(NULL, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) && (xcp_event != NULL), NULL);
        /*
         * Initialize mailbox
         * This receives packets from underlying CAN peripheral
         */
        Mailbox_Params_init(&mbxParam);
        mbxParam.readerEvent = xcp_event;
        mbxParam.readerEventId = XCP_EVENT_RX_PENDING;
        mbxCanRx = Mailbox_create(sizeof(CAN_MAILBOX_ENTRY_T), 16, &mbxParam, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) && (mbxCanRx != NULL), NULL);
        /* This mailbox is for Tx done notification */
        Mailbox_Params_init(&mbxParam);
        mbxParam.readerEvent = xcp_event;
        mbxParam.readerEventId = XCP_EVENT_TX_DONE;
        mbxCanTx = Mailbox_create(sizeof(CAN_MAILBOX_ENTRY_T), 2, &mbxParam, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) && (mbxCanTx != NULL), NULL);
        /*
         * Configure underlying CAN mailbox
         */
        /* Transmit */
        retBool = CAN_tx_config(CAN_A, CONFIG_XCP_CAN_TX_MB, 16);
        Assert_isTrue(retBool, NULL);
        retBool = CAN_mailbox_register(CAN_A, CONFIG_XCP_CAN_TX_MB, mbxCanTx);
        Assert_isTrue(retBool, NULL);
        /* Receive */
        retBool = CAN_rx_config(CAN_A, CONFIG_XCP_CAN_RX_MB, CONFIG_XCP_RX_CAN_ID, 0xF800, FALSE, mbxCanRx);
        Assert_isTrue(retBool, NULL);

        /*
         * Periodic Clock
         */
        Clock_Params_init(&clkParam);
        clkParam.period = CONFIG_XCP_PROC_INTERVAL_MS;
        clkParam.startFlag = FALSE;
        xcpClkHandle = Clock_create(XcpClkFxn, CONFIG_XCP_PROC_INTERVAL_MS, &clkParam, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) && (xcpClkHandle != NULL), NULL);

        /*
         * Task
         */
        Task_Params_init(&tskParam);
        tskParam.priority = CONFIG_XCP_TASK_PRIORITY;
        tskParam.stackSize = CONFIG_XCP_TASK_STACK;
        xcpTaskHandle = Task_create(XcpTaskFxn, &tskParam, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) && (xcpTaskHandle != NULL), NULL);

        bInitDone = TRUE;
    }
}


// Transmit a XCP packet
// Called by the XCP driver
void ApplXcpSend(vuint8 len, MEMORY_ROM BYTEPTR msg)
{
    CAN_MAILBOX_ENTRY_T txCan;
    Uint16 i = 0;

    if(len > sizeof(txCan.data)) {
#if defined(XCP_ENABLE_TESTMODE)
        System_printf("ApplXcpSend: %d exceeds max %d\n", len, sizeof(txCan.data));
#endif
        return;
    }
    txCan.msgId = CONFIG_XCP_TX_CAN_ID;
    txCan.mailboxNumber = CONFIG_XCP_CAN_TX_MB;
    txCan.len = len;
    txCan.rtr = 0;
    for (i = 0; i < len; i++) {
        txCan.data[i] = msg[i];
    }
    CAN_send(CAN_A, &txCan);
}


vuint8 ApplXcpSendStall(void) {
    while(!CAN_send_done(CAN_A, CONFIG_XCP_CAN_TX_MB)) {
        Task_sleep(1);
    }
    XcpSendCallBack();

    return 1;
}


// Convert a XCP address to a pointer
MTABYTEPTR ApplXcpGetPointer( vuint8 addr_ext, vuint32 addr )
{
    (void)addr_ext;  //unused
    return (MTABYTEPTR)addr;
}

#if defined (XCP_ENABLE_TESTMODE)
void SysPutch(char ch)
{
    UART_send(UART_A, &ch, 1);
}
#endif

#if defined (XCP_ENABLE_DAQ_TIMESTAMP)

XcpDaqTimestampType ApplXcpGetTimestamp( void )
{
    return ((XcpDaqTimestampType)Clock_getTicks());
}

void ApplXcpTimestampInit(void)
{
    // Do nothing as it uses the SysBIOS CLOCK tick
}

#endif /* XCP_ENABLE_DAQ_TIMESTAMP */


static void XcpTaskFxn(UArg a0, UArg a1)
{
    UInt posted = 0;
    UInt16 event_prescaler = 0;
    CAN_MAILBOX_ENTRY_T canMsg;

    XcpInit();

    Clock_start(xcpClkHandle);

    while(1) {
        posted = Event_pend(xcp_event, 0,
                    (XCP_EVENT_PROCESS | XCP_EVENT_TX_DONE | XCP_EVENT_RX_PENDING),
                    BIOS_WAIT_FOREVER);

        if(posted & XCP_EVENT_PROCESS) {
            /* 10ms event */
            XcpEvent(XCP_EVENT_10MS);
            event_prescaler++;
            if(event_prescaler >= 10) {
                event_prescaler = 0;
                XcpEvent(XCP_EVENT_100MS);
            }
        }

        if(posted & XCP_EVENT_TX_DONE) {
            while(Mailbox_pend(mbxCanTx, &canMsg, BIOS_NO_WAIT)) {
                if(canMsg.evtId == CAN_MAILBOX_TRANSMIT_DONE_EVT) {
                    XcpSendCallBack();
                }
            }
        }

        if(posted & XCP_EVENT_RX_PENDING) {
            while(Mailbox_pend(mbxCanRx, &canMsg, BIOS_NO_WAIT)) {
                if(canMsg.evtId == CAN_MAILBOX_RECEIVE_EVT) {
                    XcpCommand((void *)(canMsg.data));
                }
            }
        }
    }
}


static void XcpClkFxn(UArg a)
{
    Event_post(xcp_event, XCP_EVENT_PROCESS);
}
