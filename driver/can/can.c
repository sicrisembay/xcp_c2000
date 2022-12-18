/*!
 * \file can.c
 */

#include "driver_def.h"

#if CONFIG_USE_CAN

#include <stdbool.h>
#include "string.h"  /* for memset */
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/family/c28/Hwi.h>
#include "f2833x/v142/DSP2833x_headers/include/DSP2833x_Device.h"
#include "can.h"


/*!
 * \def MBOX_REG(id, num)
 * Helper macro that get the pointer to a Mailbox Registers specified by \a id and \a num
 */
#define MBOX_REG(id, num)       (struct MBOX *)(&((&(CAN_MBOX[id]->MBOX0))[num]))

volatile struct ECAN_REGS * CAN_REG[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    &ECanaRegs,
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    &ECanbRegs
#endif /* CONFIG_ENABLE_CAN_B */
};


volatile struct LAM_REGS * CAN_LAM[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    &ECanaLAMRegs,
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    &ECanbLAMRegs
#endif /* CONFIG_ENABLE_CAN_B */
};


volatile struct ECAN_MBOXES * CAN_MBOX[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    &ECanaMboxes,
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    &ECanbMboxes
#endif /* CONFIG_ENABLE_CAN_B */
};


static UInt16 const CAN_BTC_BRP[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    CONFIG_CAN_A_BRP,
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    CONFIG_CAN_B_BRP
#endif /* CONFIG_ENABLE_CAN_B */
};

static UInt16 const CAN_BTC_TSEG1[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    CONFIG_CAN_A_TSEG1,
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    CONFIG_CAN_B_TSEG1
#endif /* CONFIG_ENABLE_CAN_B */
};


static UInt16 const CAN_BTC_TSEG2[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    CONFIG_CAN_A_TSEG2,
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    CONFIG_CAN_B_TSEG2
#endif /* CONFIG_ENABLE_CAN_B */
};


static UInt32 const CAN_MC_DBO[N_USE_CAN] = {
#if CONFIG_ENABLE_CAN_A
    #if defined(CONFIG_CAN_A_DBO_LSB)
        0,
    #else
        1,
    #endif
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
    #if defined(CONFIG_CAN_B_DBO_LSB)
        0,
    #else
        1,
    #endif
#endif /* CONFIG_ENABLE_CAN_B */
};


static bool bInit = false;
static Hwi_Handle hwiHdl_can[N_USE_CAN] = {NULL};
static Mailbox_Handle mailboxTable[N_USE_CAN][32] = {NULL};
static Mailbox_Handle mailboxTxTable[N_USE_CAN][32] = {NULL};
volatile UInt16 nestIntCnt[N_USE_CAN] = {0};

static void CAN_disable_global_int(CAN_PERIPH_ID_T id);
static void CAN_enable_global_int(CAN_PERIPH_ID_T id);
static void CAN_disable_mailbox_int(CAN_PERIPH_ID_T id, UInt16 mailboxNumber);
static void CAN_enable_mailbox_int(CAN_PERIPH_ID_T id, UInt16 mailboxNumber);
static Void can_hwi_handler(UArg arg);


/*!
 * \page page_can
 * \subsection subsect_can_init CAN_init
 * <PRE>void CAN_init(void);</PRE>
 *
 * \fn void CAN_init(void)
 *
 * This function initializes Controller Area Network (CAN).
 *
 * \param None
 *
 * \return None
 */
void CAN_init(void)
{
    struct ECAN_REGS ecanShadow;
    CAN_PERIPH_ID_T id;
    Error_Block eb;
    Hwi_Params hwiParams;

    /* Guard against multiple initialization */
    if(bInit == false) {
        /*
         * Clear mailbox table
         */
        memset(mailboxTable, 0, sizeof(mailboxTable));
        memset(mailboxTxTable, 0, sizeof(mailboxTxTable));

        /*
         * Hwi
         */
#if CONFIG_ENABLE_CAN_A
        Hwi_Params_init(&hwiParams);
        hwiParams.enableAck = true;
        hwiParams.arg = CAN_A;
        hwiHdl_can[CAN_A] = Hwi_create(100, can_hwi_handler, &hwiParams, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) &&
                (hwiHdl_can[CAN_A] != NULL), NULL);
#endif /* CONFIG_ENABLE_CAN_A */
#if CONFIG_ENABLE_CAN_B
        Hwi_Params_init(&hwiParams);
        hwiParams.enableAck = true;
        hwiParams.arg = CAN_B;
        hwiHdl_can[CAN_B] = Hwi_create(102, can_hwi_handler, &hwiParams, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) &&
                (hwiHdl_can[CAN_B] != NULL), NULL);
#endif /* CONFIG_ENABLE_CAN_B */

        /*
         * Initialize communication structure and underlying peripheral
         */
        EALLOW;
#if CONFIG_ENABLE_CAN_A
        /* CAN-A pin setting */
#if CONFIG_CAN_A_TX_GPIO19
        GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;     //Enable pull-up for GPIO19 (CANTXA)
        GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 3;    // Configure GPIO19 for CANTXA
#elif CONFIG_CAN_A_TX_GPIO31
        GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;     //Enable pull-up for GPIO31 (CANTXA)
        GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;    // Configure GPIO31 for CANTXA
#else
#error "Invalid CAN-A Tx GPIO"
#endif

#if CONFIG_CAN_A_RX_GPIO18
        GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;     // Enable pull-up for GPIO18 (CANRXA)
        GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3;   // Asynch qual for GPIO18 (CANRXA)
        GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 3;    // Configure GPIO18 for CANRXA
#elif CONFIG_CAN_A_RX_GPIO30
        GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;     // Enable pull-up for GPIO30 (CANRXA)
        GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;   // Asynch qual for GPIO30 (CANRXA)
        GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;    // Configure GPIO30 for CANRXA
#else
#error "Invalid CAN-A Rx GPIO"
#endif
        /* Enable CAN-A clock */
        SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;
#endif /* CONFIG_ENABLE_CAN_A */

#if CONFIG_ENABLE_CAN_B
#if CONFIG_CANB_TX_GPIO8
        GpioCtrlRegs.GPAPUD.bit.GPIO8 = 0;      // Enable pull-up for GPIO8(CANTXB)
        GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 2;     // Configure GPIO8 for CANTXB
#elif CONFIG_CANB_TX_GPIO12
        GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0;      // Enable pull-up for GPIO12(CANTXB)
        GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 2;     // Configure GPIO12 for CANTXB
#elif CONFIG_CANB_TX_GPIO16
        GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;      // Enable pull-up for GPIO16(CANTXB)
        GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 2;     // Configure GPIO16 for CANTXB
#elif CONFIG_CANB_TX_GPIO20
        GpioCtrlRegs.GPAPUD.bit.GPIO20 = 0;     // Enable pull-up for GPIO20(CANTXB)
        GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 3;    // Configure GPIO20 for CANTXB
#else
#error "Invalid CAN-B Tx GPIO"
#endif /* CONFIG_CANB_TX_GPIOXX */

#if CONFIG_CANB_RX_GPIO10
        GpioCtrlRegs.GPAPUD.bit.GPIO10 = 0;     // Enable pull-up for GPIO10(CANRXB)
        GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 3;   // Asynch qual for GPIO10 (CANRXB)
        GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 2;    // Configure GPIO10 for CANRXB
#elif CONFIG_CANB_RX_GPIO13
        GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;     // Enable pull-up for GPIO13(CANRXB)
        GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 3;   // Asynch qual for GPIO13 (CANRXB)
        GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 2;    // Configure GPIO13 for CANRXB
#elif CONFIG_CANB_RX_GPIO17
        GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0;     // Enable pull-up for GPIO17(CANRXB)
        GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3;   // Asynch qual for GPIO17 (CANRXB)
        GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 2;    // Configure GPIO17 for CANRXB
#elif CONFIG_CANB_RX_GPIO21
        GpioCtrlRegs.GPAPUD.bit.GPIO21 = 0;     // Enable pull-up for GPIO21(CANRXB)
        GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 3;   // Asynch qual for GPIO21 (CANRXB)
        GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 3;    // Configure GPIO21 for CANRXB
#else
#endif /* CONFIG_CANB_RX_GPIOXX */
        SysCtrlRegs.PCLKCR0.bit.ECANBENCLK = 1;
#endif /* CONFIG_ENABLE_CAN_B */

        for (id = (CAN_PERIPH_ID_T)0; id < N_USE_CAN; id++) {
            /* Configure eCAN TX pin for can operation */
            ecanShadow.CANTIOC.all = CAN_REG[id]->CANTIOC.all;
            ecanShadow.CANTIOC.bit.TXFUNC = 1;
            CAN_REG[id]->CANTIOC.all = ecanShadow.CANTIOC.all;

            /* Configure eCAN RX pin for can operation */
            ecanShadow.CANRIOC.all = CAN_REG[id]->CANRIOC.all;
            ecanShadow.CANRIOC.bit.RXFUNC = 1;
            CAN_REG[id]->CANRIOC.all = ecanShadow.CANRIOC.all;

            /* Configure for eCAN mode and enable auto bus on */
            ecanShadow.CANMC.all = CAN_REG[id]->CANMC.all;
            ecanShadow.CANMC.bit.SCB = 1;
            ecanShadow.CANMC.bit.ABO = 1;
            CAN_REG[id]->CANMC.all = ecanShadow.CANMC.all;

            /* All bits of MSGCTRL must be initialized to zero */
            CAN_MBOX[id]->MBOX0.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX1.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX2.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX3.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX4.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX5.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX6.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX7.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX8.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX9.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX10.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX11.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX12.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX13.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX14.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX15.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX16.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX17.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX18.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX19.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX20.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX21.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX22.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX23.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX24.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX25.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX26.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX27.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX28.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX29.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX30.MSGCTRL.all = 0x00000000;
            CAN_MBOX[id]->MBOX31.MSGCTRL.all = 0x00000000;

            /* All Global and Mailbox interrupts are generated in INT Line0 */
            CAN_REG[id]->CANGIM.all = 0x00000000;
            CAN_REG[id]->CANMIM.all = 0x00000000;
            CAN_REG[id]->CANMIL.all = 0x00000000;

            /* Clear all TAn, RMPn, and interrupt flags */
            CAN_REG[id]->CANTA.all = 0xFFFFFFFF;
            CAN_REG[id]->CANRMP.all = 0xFFFFFFFF;
            CAN_REG[id]->CANGIF0.all = 0xFFFFFFFF;
            CAN_REG[id]->CANGIF1.all = 0xFFFFFFFF;

            /* Configure bit timing */
            ecanShadow.CANMC.all = CAN_REG[id]->CANMC.all;
            ecanShadow.CANMC.bit.CCR = 1;
            CAN_REG[id]->CANMC.all = ecanShadow.CANMC.all;
            ecanShadow.CANES.all = CAN_REG[id]->CANES.all;
            do {
                ecanShadow.CANES.all = CAN_REG[id]->CANES.all;
            } while(ecanShadow.CANES.bit.CCE != 1);

            /* Configure CAN Baudrate */
            ecanShadow.CANBTC.all = 0;
            ecanShadow.CANBTC.bit.BRPREG = CAN_BTC_BRP[id];
            ecanShadow.CANBTC.bit.TSEG1REG = CAN_BTC_TSEG1[id];
            ecanShadow.CANBTC.bit.TSEG2REG = CAN_BTC_TSEG2[id];
            ecanShadow.CANBTC.bit.SAM = 1;
            CAN_REG[id]->CANBTC.all = ecanShadow.CANBTC.all;

            ecanShadow.CANMC.all = CAN_REG[id]->CANMC.all;
            ecanShadow.CANMC.bit.CCR = 0;
            CAN_REG[id]->CANMC.all = ecanShadow.CANMC.all;
            ecanShadow.CANES.all = CAN_REG[id]->CANES.all;
            do {
                ecanShadow.CANES.all = CAN_REG[id]->CANES.all;
            } while(ecanShadow.CANES.bit.CCE != 0);

            /* Configure CAN message data byte order */
            ecanShadow.CANMC.all = CAN_REG[id]->CANMC.all;
            ecanShadow.CANMC.bit.DBO = CAN_MC_DBO[id];
            CAN_REG[id]->CANMC.all = ecanShadow.CANMC.all;

            /* Disable all mailbox as default */
            CAN_REG[id]->CANME.all = 0x00000000;

            /* Enable Global Interrupt */
            ecanShadow.CANGIM.all = CAN_REG[id]->CANGIM.all;
            ecanShadow.CANGIM.bit.I0EN = 1;
            CAN_REG[id]->CANGIM.all = ecanShadow.CANGIM.all;
            nestIntCnt[id] = 0;
        }
        EDIS;

#if CONFIG_ENABLE_CLI_CAN_COMMAND
        CMD_CAN_init();
#endif
        bInit = true;
    }
}


/*!
 * \page page_can
 * \subsection subsect_can_init_done CAN_init_done
 * <PRE>Bool CAN_init_done(void);</PRE>
 *
 * \fn Bool CAN_init_done(void)
 *
 * This function returns the initialization state CAN peripheral.
 *
 * \param None
 *
 * \return true: init done; false: otherwise
 */
Bool CAN_init_done(void)
{
    return bInit;
}


static void CAN_disable_global_int(CAN_PERIPH_ID_T id)
{
    union CANGIM_REG shadow;

    EALLOW;
    shadow.all = CAN_REG[id]->CANGIM.all;
    shadow.bit.I0EN = 0;
    CAN_REG[id]->CANGIM.all = shadow.all;
    EDIS;
    nestIntCnt[id]++;
}


static void CAN_enable_global_int(CAN_PERIPH_ID_T id)
{
    union CANGIM_REG shadow;

    nestIntCnt[id]--;
    if(nestIntCnt[id] == 0) {
        EALLOW;
        shadow.all = CAN_REG[id]->CANGIM.all;
        shadow.bit.I0EN = 1;
        CAN_REG[id]->CANGIM.all = shadow.all;
        EDIS;
    }
}


static void CAN_disable_mailbox_int(CAN_PERIPH_ID_T id, UInt16 mailboxNumber)
{
    UInt32 u32MIM;

    if(mailboxNumber >= 32) {
        /* Valid value is 0-31, inclusive */
        return;
    }

    CAN_disable_global_int(id);
    EALLOW;
    u32MIM = CAN_REG[id]->CANMIM.all;
    u32MIM &= ~(1UL << mailboxNumber);
    CAN_REG[id]->CANMIM.all = u32MIM;
    EDIS;
    CAN_enable_global_int(id);
}


static void CAN_enable_mailbox_int(CAN_PERIPH_ID_T id, UInt16 mailboxNumber)
{
    UInt32 u32MIM;

    if(mailboxNumber >= 32) {
        /* Valid value is 0-31, inclusive */
        return;
    }

    CAN_disable_global_int(id);
    EALLOW;
    u32MIM = CAN_REG[id]->CANMIM.all;
    u32MIM |= (1UL << mailboxNumber);
    CAN_REG[id]->CANMIM.all = u32MIM;
    EDIS;
    CAN_enable_global_int(id);
}


/*!
 * \page page_can
 * \subsection subsect_can_rx_config CAN_rx_config
 * <PRE>Bool CAN_rx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 msgID,
 *                      Bool bIsTx, Bool overwriteProtect, Mailbox_Handle pMB);</PRE>
 *
 * \fn Bool CAN_rx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 msgIdFilter, UInt16 filterMask, Bool overwriteProtect, Mailbox_Handle pMB)
 *
 * This function configures a specified mailbox of a specified CAN peripheral.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     Mailbox number. Valid value is 0-31, inclusive
 * \param msgIdFilter       11-bit Message ID assigned to this mailbox
 * \param filterMask        11-bit Acceptance (bit-wise) filter for incoming message ID. Set bit is don't care.
 * \param overwriteProtect  Mailbox overwrite protection. Set to true to enable protection.
 * \param pMB               SysBIOS mailbox handle for event and data notification
 *
 * \return true: successful; false: otherwise
 */
Bool CAN_rx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 msgIdFilter, UInt16 filterMask, Bool overwriteProtect, Mailbox_Handle pMB)
{
    UInt32 u32Mask = 0;
    struct MBOX * pMBox;
    UInt32 shadowCANME;
    UInt32 shadowCANMD;
    UInt32 shadowCANOPC;
    union CANMSGID_REG shadowMSGID;
    union CANLAM_REG shadowLAM;
    union CANLAM_REG * pLAM = (union CANLAM_REG *)(&(CAN_LAM[id]->LAM0));

    if((id >= N_USE_CAN) || (mailboxNumber >= 32)) {
        /* Invalid argument */
        return false;
    }

    if(bInit == 0) {
        /* Must be initialized first */
        return false;
    }

    /* Disable interrupt to prevent race condition during configuration */
    /* CAN critical section start --> */
    CAN_disable_global_int(id);

    /* Disable mailbox before configuration */
    shadowCANME = CAN_REG[id]->CANME.all;
    shadowCANME &= ~(1UL << mailboxNumber);
    CAN_REG[id]->CANME.all = shadowCANME;

    /* Set mailbox MSGID filter with Acceptance filter enabled */
    shadowMSGID.all = 0;
    shadowMSGID.bit.STDMSGID = msgIdFilter;
    shadowMSGID.bit.AME = 1;
    pMBox = MBOX_REG(id, mailboxNumber);
    pMBox->MSGID.all = shadowMSGID.all;

    /* Set local acceptance mask */
    u32Mask = (UInt32)(((UInt32)filterMask << 18) & 0x1FFFFFFF);
    shadowLAM.all = 0;
    shadowLAM.bit.LAMI = 1;
    shadowLAM.bit.LAM_H = (u32Mask >> 16) & 0x1FFF;
    shadowLAM.bit.LAM_L = (u32Mask & 0xFFFF);
    pLAM[mailboxNumber].all = shadowLAM.all;

    /* Configure mailbox overwrite Protection */
    shadowCANOPC = CAN_REG[id]->CANOPC.all;
    if(overwriteProtect) {
        shadowCANOPC |= (1UL << mailboxNumber);
    } else {
        shadowCANOPC &= ~(1UL << mailboxNumber);
    }
    CAN_REG[id]->CANOPC.all = shadowCANOPC;

    /* Configure mailbox direction */
    shadowCANMD = CAN_REG[id]->CANMD.all;
    shadowCANMD |= (1UL << mailboxNumber);
    CAN_REG[id]->CANMD.all = shadowCANMD;

    CAN_mailbox_register(id, mailboxNumber, pMB);

    /* Enable Mailbox */
    shadowCANME = CAN_REG[id]->CANME.all;
    shadowCANME |= (1UL << mailboxNumber);
    CAN_REG[id]->CANME.all = shadowCANME;

    /* <-- end of CAN critical section */
    CAN_enable_global_int(id);

    return(true);
}


/*!
 * \page page_can
 * \subsection subsect_can_tx_config CAN_tx_config
 * <PRE>Bool CAN_tx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 mailboxSize);</PRE>
 *
 * \fn Bool CAN_tx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 mailboxSize);
 *
 * This function allocate a SysBios mailbox to specified CAN \a id and mailbox number.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     Mailbox number. Valid value is 0-31, inclusive
 * \param mailboxSize       Mailbox size
 *
 * \return true: successful; false: otherwise
 */
Bool CAN_tx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 mailboxSize)
{
    Mailbox_Params mbxParams;
    Error_Block eb;

    if((id >= N_USE_CAN) || (mailboxNumber >= 32) || (mailboxSize == 0)) {
        /* Invalid argument */
        return false;
    }

    if(bInit == 0) {
        /* Must be initialized first */
        return false;
    }

    /*
     * Create mailbox that will used to buffer transmission packets
     */
    if(mailboxTxTable[id][mailboxNumber] != NULL) {
        /*
         * Mailbox has been already created.  This is not expected.
         * Return false to indicate to caller.
         */
        return false;
    } else {
        Error_init(&eb);
        /* Create buffer used for transmission */
        Mailbox_Params_init(&mbxParams);
        mailboxTxTable[id][mailboxNumber] = Mailbox_create(sizeof(CAN_MAILBOX_ENTRY_T), mailboxSize, &mbxParams, &eb);
        Assert_isTrue((Error_check(&eb) == FALSE) && (mailboxTxTable[id][mailboxNumber] != NULL), NULL);
        /* Enable Mailbox Interrupt */
        CAN_enable_mailbox_int(id, mailboxNumber);
    }

    return(true);
}


/*!
 * \page page_can
 * \subsection subsect_can_mailbox_register CAN_mailbox_register
 * <PRE>Bool CAN_mailbox_register(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, Mailbox_Handle pMB);</PRE>
 *
 * \fn Bool CAN_mailbox_register(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, Mailbox_Handle pMB)
 *
 * This function registers a SysBIOS mailbox handle to specified \a mailboxNumber of a specified peripheral \a id.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     Mailbox number. Valid value is 0-31, inclusive
 * \param pMB               SysBIOS mailbox handle for event and data notification
 *
 * \return true: successful; false: otherwise */
Bool CAN_mailbox_register(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, Mailbox_Handle pMB)
{
    if((id >= N_USE_CAN) || (mailboxNumber >= 32)) {
        /* Invalid argument */
        return false;
    }

    if(bInit == 0) {
        /* Must be initialized first */
        return false;
    }

    /* Register Mailbox Handle */
    mailboxTable[id][mailboxNumber] = pMB;
    if(pMB != NULL) {
        /* Enable mailbox interrupt */
        CAN_enable_mailbox_int(id, mailboxNumber);
    } else {
        /* Disable mailbox interrupt */
        CAN_disable_mailbox_int(id, mailboxNumber);
    }
    return true;
}


/*!
 * \page page_can
 * \subsection subsect_can_send CAN_send
 * <PRE>Bool CAN_mailbox_send(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 * pMsg, UInt16 len);</PRE>
 *
 * \fn Bool CAN_send(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket)
 *
 * This function sends message to a specified mailbox of a specified CAN peripheral.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param pMsg              Pointer to CAN packet to send
 *
 * \return true: successful; false: otherwise
 */
Bool CAN_send(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket)
{
    struct MBOX * pMBox;
    struct MBOX shadowMBOX;         /* Shadow of MBOX */
    union CANME_REG shadowCANME;    /* Shadow of CANME */
    UInt32 u32MD;                   /* Shadow of CANMD */
    UInt32 u32TRS;                  /* Shadow of TRS */
    union CANGIF0_REG shadowCANGIF0; /* Shadow of CANGIF0 */
    UInt16 mailboxNumber;
    Bool retval = true;
    static Uint16 bBusOff[N_USE_CAN] =
        { 0 };

    if((id >= N_USE_CAN) || (pCanPacket == NULL)) {
        /* Invalid Argument */
        return false;
    }

    if((pCanPacket->mailboxNumber >= 32) || (pCanPacket->len > 8)) {
        /* Invalid Packet */
        return false;
    }

    mailboxNumber = pCanPacket->mailboxNumber;

    if(bInit == 0) {
        /* Must be initialized first */
        return false;
    }

    shadowCANGIF0.all = CAN_REG[id]->CANGIF0.all;
    if (shadowCANGIF0.bit.BOIF0) {
        /* Bus OFF */
        if (bBusOff[id] != shadowCANGIF0.bit.BOIF0) {
            /* Send only once */
#if CONFIG_USE_CLI
#if CONFIG_CLI_IO_UART
            System_printf("CAN%d Bus off!\r\n", (int) id);
#elif CONFIG_CLI_IO_CAN
            /// TODO!
#endif
#endif
        }
    } else {
        if (bBusOff[id] != shadowCANGIF0.bit.BOIF0) {
            /* Send only once */
#if CONFIG_USE_CLI
#if CONFIG_CLI_IO_UART
            System_printf("CAN%d Bus on.\r\n", (int) id);
#elif CONFIG_CLI_IO_CAN
            /// TODO!
#endif
#endif
        }
    }

    bBusOff[id] = shadowCANGIF0.bit.BOIF0;

    if (bBusOff[id]) {
        return false;
    }

    /*
     * Prevent Race Condition
     * Note: ISR can preempt while posting to SysBIOS mailbox and
     * we don't want that.  Thus, disable CAN ISR to prevent race
     * condition.
     */
    /* CAN critical section start --> */
    CAN_disable_global_int(id);
    u32TRS = CAN_REG[id]->CANTRS.all;

    if (((u32TRS & (1UL << mailboxNumber)) != 0)
            || (0 != Mailbox_getNumPendingMsgs(mailboxTxTable[id][mailboxNumber]))) {
        /*
         * On-going transmission
         * Push to mailbox
         */
        if(mailboxTxTable[id][mailboxNumber] == NULL) {
            /* Null mailbox */
            retval = false;
        } else {
            if(TRUE != Mailbox_post(mailboxTxTable[id][mailboxNumber], pCanPacket, BIOS_NO_WAIT)) {
                /* Failed to post (Mailbox is FULL) */
                retval = false;
            }
        }
    } else {
        /* Disable mailbox to be able to modify MSGID */
        shadowCANME.all = CAN_REG[id]->CANME.all;
        shadowCANME.all &= ~(1UL << mailboxNumber);
        CAN_REG[id]->CANME.all = shadowCANME.all;
        /* Set MSGID */
        shadowMBOX.MSGID.all = 0;
        shadowMBOX.MSGID.bit.STDMSGID = pCanPacket->msgId;
        pMBox = MBOX_REG(id, mailboxNumber);
        pMBox->MSGID.all = shadowMBOX.MSGID.all;

        u32MD = CAN_REG[id]->CANMD.all;
        u32MD &= ~(1UL << mailboxNumber);
        CAN_REG[id]->CANMD.all = u32MD;

        shadowMBOX.MSGCTRL.all = 0x00000000;
        shadowMBOX.MSGCTRL.bit.DLC = pCanPacket->len;
        shadowMBOX.MDL.byte.BYTE0 = pCanPacket->data[0];
        shadowMBOX.MDL.byte.BYTE1 = pCanPacket->data[1];
        shadowMBOX.MDL.byte.BYTE2 = pCanPacket->data[2];
        shadowMBOX.MDL.byte.BYTE3 = pCanPacket->data[3];
        shadowMBOX.MDH.byte.BYTE4 = pCanPacket->data[4];
        shadowMBOX.MDH.byte.BYTE5 = pCanPacket->data[5];
        shadowMBOX.MDH.byte.BYTE6 = pCanPacket->data[6];
        shadowMBOX.MDH.byte.BYTE7 = pCanPacket->data[7];

        pMBox->MSGCTRL.all = shadowMBOX.MSGCTRL.all;
        pMBox->MDL.all = shadowMBOX.MDL.all;
        pMBox->MDH.all = shadowMBOX.MDH.all;

        /* Re-enable mailbox */
        shadowCANME.all = CAN_REG[id]->CANME.all;
        shadowCANME.all |= (1UL << mailboxNumber);
        CAN_REG[id]->CANME.all = shadowCANME.all;

        u32TRS |= (1UL << mailboxNumber);
        CAN_REG[id]->CANTRS.all = u32TRS;
    }
    /* <-- end of CAN critical section */
    CAN_enable_global_int(id);

    return (retval);
}


/*!
 * \page page_can
 * \subsection subsect_can_send_done CAN_send_done
 * <PRE>Bool CAN_send_done(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket);</PRE>
 *
 * \fn Bool CAN_send_done(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket)
 *
 * This function returns true if all CAN tx in the mailbox has been sent.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     CAN mailbox number used for transmission
 *
 * \return true: All messages are sent; false: otherwise
 */
Bool CAN_send_done(CAN_PERIPH_ID_T id, UInt16 mailboxNumber)
{
    Bool retval = false;
    UInt32 u32TRS;

    if((id >= N_USE_CAN) || (mailboxNumber >= 32)) {
        /* Invalid Argument */
        return false;
    }

    if(mailboxTxTable[id][mailboxNumber] == NULL) {
        /* transmit mailbox has not been initialized */
        return false;
    }

    u32TRS = CAN_REG[id]->CANTRS.all;

    if(((u32TRS & (1UL << mailboxNumber)) == 0) &&
       (Mailbox_getNumPendingMsgs(mailboxTxTable[id][mailboxNumber]) == 0)) {
        retval = true;
    }

    return (retval);
}


#pragma CODE_SECTION(can_hwi_handler, "ramfuncs");
static Void can_hwi_handler(UArg arg)
{
    CAN_PERIPH_ID_T id = (CAN_PERIPH_ID_T)arg;
    UInt32 u32GIF0;                         /* shadow GIF0 */
    UInt32 u32TA;                           /* shadow TA */
    UInt32 u32RMP;                          /* shadow RMP */
    UInt32 u32TRS;                          /* shadow TRS */
    UInt32 u32ME;                           /* shadow ME */
    union CANMSGCTRL_REG shadowMSGCTRL;     /* shadow MSGCTRL */
    union CANMSGID_REG shadowMSGID;         /* shadow MSGID */
    Int16 mailboxNumber = 0;
    struct MBOX * pMBox;
    CAN_MAILBOX_ENTRY_T entry;


    /* New interrupt will be serviced by the core */
    u32GIF0 = CAN_REG[id]->CANGIF0.all;
    CAN_REG[id]->CANGIF0.all = u32GIF0;

    u32TRS = CAN_REG[id]->CANTRS.all;
    /* TX interrupt */
    u32TA = CAN_REG[id]->CANTA.all & CAN_REG[id]->CANMIM.all;
    if(u32TA != 0) {
        /* clear TX acknowledge */
        CAN_REG[id]->CANTA.all = u32TA;

        /*
         * Note: Mailbox Interrupt Vector is not used. TA/RMP flags are used instead.
         * Rational: Simultaneous mailbox interrupt request can occur and
         * "for" loop is less expensive (execution) than back to back HWI
         * execution.
         */
        for(mailboxNumber = 31; mailboxNumber >= 0; mailboxNumber--) {
            if(u32TA & (1UL << mailboxNumber)) {

                if((u32TRS & (1UL << mailboxNumber)) == 0) {
                    /* transmit any pending message */
                    if(Mailbox_pend(mailboxTxTable[id][mailboxNumber], &entry, BIOS_NO_WAIT)) {
                        /* Get pointer to mailbox */
                        pMBox = MBOX_REG(id, mailboxNumber);
                        /* Disable mailbox to be able to modify MSGID */
                        u32ME = CAN_REG[id]->CANME.all;
                        u32ME &= ~(1UL << mailboxNumber);
                        CAN_REG[id]->CANME.all = u32ME;
                        /* Set MSGID */
                        shadowMSGID.all = 0x00000000;
                        shadowMSGID.bit.STDMSGID = entry.msgId;
                        pMBox->MSGID.all = shadowMSGID.all;
                        /* Transmit payload */
                        shadowMSGCTRL.all = pMBox->MSGCTRL.all;
                        shadowMSGCTRL.bit.DLC = entry.len;
                        pMBox->MSGCTRL.all = shadowMSGCTRL.all;
                        pMBox->MDL.byte.BYTE0 = entry.data[0];
                        pMBox->MDL.byte.BYTE1 = entry.data[1];
                        pMBox->MDL.byte.BYTE2 = entry.data[2];
                        pMBox->MDL.byte.BYTE3 = entry.data[3];
                        pMBox->MDH.byte.BYTE4 = entry.data[4];
                        pMBox->MDH.byte.BYTE5 = entry.data[5];
                        pMBox->MDH.byte.BYTE6 = entry.data[6];
                        pMBox->MDH.byte.BYTE7 = entry.data[7];
                        /* Enable mailbox */
                        u32ME = CAN_REG[id]->CANME.all;
                        u32ME |= (1UL << mailboxNumber);
                        CAN_REG[id]->CANME.all = u32ME;
                        /* Mailbox transmit request */
                        u32TRS |= (1UL << mailboxNumber);
                        CAN_REG[id]->CANTRS.all = u32TRS;
                    }
                }

                /* invoke callback */
                if(mailboxTable[id][mailboxNumber] != NULL) {
                    memset(&entry, 0, sizeof(entry));
                    entry.evtId = CAN_MAILBOX_TRANSMIT_DONE_EVT;
                    entry.mailboxNumber = mailboxNumber;
                    if(TRUE != Mailbox_post(mailboxTable[id][mailboxNumber], &entry, BIOS_NO_WAIT)) {
                        /// TODO: Handle failure on posting to mailbox
                    }
                }
            }
        }
    }

    /* RX interrupt */
    u32RMP = CAN_REG[id]->CANRMP.all & CAN_REG[id]->CANMIM.all;
    if(u32RMP != 0) {
        /* clear Rx pending */
        CAN_REG[id]->CANRMP.all = u32RMP;

        /* invoke callback */
        for(mailboxNumber = 31; mailboxNumber >= 0; mailboxNumber--) {
            if(u32RMP & (1UL << mailboxNumber)) {
                if(mailboxTable[id][mailboxNumber] != NULL) {
                    pMBox = MBOX_REG(id, mailboxNumber);
                    memset(&entry, 0, sizeof(entry));
                    entry.evtId = CAN_MAILBOX_RECEIVE_EVT;
                    entry.mailboxNumber = mailboxNumber;
                    entry.msgId = pMBox->MSGID.bit.STDMSGID;
                    entry.data[0] = pMBox->MDL.byte.BYTE0;
                    entry.data[1] = pMBox->MDL.byte.BYTE1;
                    entry.data[2] = pMBox->MDL.byte.BYTE2;
                    entry.data[3] = pMBox->MDL.byte.BYTE3;
                    entry.data[4] = pMBox->MDH.byte.BYTE4;
                    entry.data[5] = pMBox->MDH.byte.BYTE5;
                    entry.data[6] = pMBox->MDH.byte.BYTE6;
                    entry.data[7] = pMBox->MDH.byte.BYTE7;
                    entry.len = pMBox->MSGCTRL.bit.DLC;
                    if(TRUE != Mailbox_post(mailboxTable[id][mailboxNumber], &entry, BIOS_NO_WAIT)) {
                        /// TODO: Handle failure on posting to mailbox
                    }
                }
            }
        }
    }
}

#endif /* CONFIG_USE_CAN */
