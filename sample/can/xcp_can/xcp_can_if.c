#include "XcpBasic.h"
#include "can/can.h"
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/BIOS.h>


static Mailbox_Handle mbxCanRx;

void XcpCanInit(void)
{
    Error_Block eb;
    Mailbox_Params mbxParam;
    Bool retBool = FALSE;

    CAN_init();

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

