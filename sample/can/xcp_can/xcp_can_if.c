#include "XcpBasic.h"
#include "can/can.h"
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/BIOS.h>
#include "f2833x/v142/DSP2833x_headers/include/DSP2833x_Device.h"
#if defined(XCP_ENABLE_TESTMODE)
#include "uart/uart.h"
#endif

static Mailbox_Handle mbxCanRx;

#if defined (XCP_ENABLE_DAQ_TIMESTAMP)
void ApplXcpTimestampInit(void);
#endif

void XcpCanInit(void)
{
    Error_Block eb;
    Mailbox_Params mbxParam;
    Bool retBool = FALSE;

    CAN_init();

#if defined(XCP_ENABLE_TESTMODE)
    UART_init();
#endif

#if defined (XCP_ENABLE_DAQ_TIMESTAMP)
    ApplXcpTimestampInit();
#endif

    /*
     * Initialize mailbox
     * This receives packets from underlying CAN peripheral
     */
    Mailbox_Params_init(&mbxParam);
    mbxCanRx = Mailbox_create(sizeof(CAN_MAILBOX_ENTRY_T), 16, &mbxParam, &eb);
    Assert_isTrue((Error_check(&eb) == FALSE) && (mbxCanRx != NULL), NULL);
    /*
     * Configure underlying CAN mailbox
     */
    /* Transmit */
    retBool = CAN_tx_config(CAN_A, CONFIG_XCP_CAN_TX_MB, 16);
    Assert_isTrue(retBool, NULL);
    /* Receive */
    retBool = CAN_rx_config(CAN_A, CONFIG_XCP_CAN_RX_MB, CONFIG_XCP_RX_CAN_ID, 0xF800, FALSE, mbxCanRx);
    Assert_isTrue(retBool, NULL);
}


void XcpHandler(void)
{
    CAN_MAILBOX_ENTRY_T rxCan;

    /* Check if the transmit has been done */
    if(CAN_send_done(CAN_A, CONFIG_XCP_CAN_TX_MB)) {
        XcpSendCallBack();
    }

    /* Handle Receive CAN */
    while(Mailbox_pend(mbxCanRx, &rxCan, BIOS_NO_WAIT)) {
        XcpCommand((void *)(rxCan.data));
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
    return ((XcpDaqTimestampType)(0xFFFFFFFF - CpuTimer0Regs.TIM.all));  // CpuTimer counts down.
}

void ApplXcpTimestampInit(void)
{
    vuint16 prescaler = (CONFIG_SYSTEM_FREQ_MHZ - 1);
    CpuTimer0Regs.TCR.bit.TSS = 1;
    CpuTimer0Regs.PRD.all = 0xFFFFFFFF;
    EALLOW;
    CpuTimer0Regs.TPR.bit.TDDR = prescaler & 0xFF;
    CpuTimer0Regs.TPRH.bit.TDDRH = (prescaler >> 8) & 0xFF;
    EDIS;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.SOFT = 1;
    CpuTimer0Regs.TCR.bit.FREE = 1;
    CpuTimer0Regs.TCR.bit.TSS = 0;
}
#endif
