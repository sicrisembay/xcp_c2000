#include "driver_def.h"
#include "string.h"  /* for memset */
#include <stdbool.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <ti/sysbios/family/c28/Hwi.h>
#include "f2833x/v142/DSP2833x_headers/include/DSP2833x_Device.h"
#include "utility/ringbuf/ringbuffer.h"
#include "uart.h"

#define SCI_CLKIN_FREQ_HZ   ((CONFIG_SYSTEM_FREQ_MHZ * 1000000U) / 4U)

#define CONCAT(x, y)        x##y
#define CONCAT_L1(x, y)     CONCAT(x, y)

#define UARTA_TX_CONFIG     GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0; \
                            GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1
#define UARTA_RX_CONFIG     GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0; \
                            GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3; \
                            GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1

#define UARTB_TX_CONFIG     GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0; \
                            GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 2
#define UARTB_RX_CONFIG     GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0; \
                            GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3; \
                            GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 2

#define UARTC_TX_CONFIG     GpioCtrlRegs.GPBPUD.bit.GPIO63 = 0; \
                            GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 1
#define UARTC_RX_CONFIG     GpioCtrlRegs.GPBPUD.bit.GPIO62 = 0; \
                            GpioCtrlRegs.GPBQSEL2.bit.GPIO62 = 3; \
                            GpioCtrlRegs.GPBMUX2.bit.GPIO62 = 1


static UInt16 const DEFAULT_HWI_INT_NUM[N_UART][2] = {
#if (CONFIG_ENABLE_UARTA)
    {
        97,     // TX
        96      // RX
    },
#endif
#if (CONFIG_ENABLE_UARTB)
    {
        99,     // TX
        98      // RX
    },
#endif
#if (CONFIG_ENABLE_UARTC)
    {
        93,     // TX
        92      // RX
    },
#endif
};


static UInt16 const DEFAULT_UART_BRR[N_UART] = {
#if (CONFIG_ENABLE_UARTA)
    ((SCI_CLKIN_FREQ_HZ / (CONFIG_BAUDRATE_UARTA * 8)) - 1),
#endif
#if (CONFIG_ENABLE_UARTB)
    ((SCI_CLKIN_FREQ_HZ / (CONFIG_BAUDRATE_UARTB * 8)) - 1),
#endif
#if (CONFIG_ENABLE_UARTC)
    ((SCI_CLKIN_FREQ_HZ / (CONFIG_BAUDRATE_UARTC * 8)) - 1),
#endif
};


static const char * tag[N_UART] = {
#if (CONFIG_ENABLE_UARTA)
    "uart-a",
#endif
#if (CONFIG_ENABLE_UARTB)
    "uart-b",
#endif
#if (CONFIG_ENABLE_UARTC)
    "uart-c",
#endif
};


static Bool bInit = false;
static Hwi_Handle hwiHdl_uart_tx[N_UART] = {NULL};
static Hwi_Handle hwiHdl_uart_rx[N_UART] = {NULL};
static ring_buffer_t ringBuf_tx[N_UART];
static ring_buffer_t ringBuf_rx[N_UART];
#if (CONFIG_ENABLE_UARTA)
static Char uarta_buffer_tx[CONFIG_TX_QUEUE_BUFF_SZ_UARTA];
static Char uarta_buffer_rx[CONFIG_RX_QUEUE_BUFF_SZ_UARTA];
#endif
#if (CONFIG_ENABLE_UARTB)
static Char uartb_buffer_tx[CONFIG_TX_QUEUE_BUFF_SZ_UARTB];
static Char uartb_buffer_rx[CONFIG_RX_QUEUE_BUFF_SZ_UARTB];
#endif
#if (CONFIG_ENABLE_UARTC)
static Char uartc_buffer_tx[CONFIG_TX_QUEUE_BUFF_SZ_UARTC];
static Char uartc_buffer_rx[CONFIG_RX_QUEUE_BUFF_SZ_UARTC];
#endif
volatile struct SCI_REGS * SCI_REG_PTR[N_UART] = {
#if (CONFIG_ENABLE_UARTA)
    &SciaRegs,
#endif
#if (CONFIG_ENABLE_UARTB)
    &ScibRegs,
#endif
#if (CONFIG_ENABLE_UARTC)
    &ScicRegs,
#endif
};


static Void uart_tx_hwi_handler(UArg arg);
static Void uart_rx_hwi_handler(UArg arg);


#pragma CODE_SECTION(uart_tx_hwi_handler, "ramfuncs");
static Void uart_tx_hwi_handler(UArg arg)
{
    UART_ID_T id = (UART_ID_T)arg;
    Char txChar;
    UInt16 fifoTxCnt = SCI_REG_PTR[id]->SCIFFTX.bit.TXFFST;

    if(ring_buffer_is_empty(&(ringBuf_tx[id])) && (fifoTxCnt == 0)) {
        /* Disable TX FIFO interrupt */
        SCI_REG_PTR[id]->SCIFFTX.bit.TXFFIENA = 0;
    } else {
        while((fifoTxCnt < 16) &&
              (!ring_buffer_is_empty(&(ringBuf_tx[id])))) {
            ring_buffer_dequeue(&(ringBuf_tx[id]), &txChar);
            SCI_REG_PTR[id]->SCITXBUF = txChar;
            fifoTxCnt = SCI_REG_PTR[id]->SCIFFTX.bit.TXFFST;
        }
    }

    /* Clear Interrupt Flag */
    SCI_REG_PTR[id]->SCIFFTX.bit.TXFFINTCLR = 1;
}


#pragma CODE_SECTION(uart_rx_hwi_handler, "ramfuncs");
static Void uart_rx_hwi_handler(UArg arg)
{
    UART_ID_T id = (UART_ID_T)arg;
    Char rxChar;
    UInt16 rxCount;
    UInt16 i;

    rxCount = SCI_REG_PTR[id]->SCIFFRX.bit.RXFFST;
    if(rxCount > 0) {
        for(i = 0; i < rxCount; i++) {
            rxChar = SCI_REG_PTR[id]->SCIRXBUF.bit.RXDT;
            /*
             * Send to rx ring buffer
             * Note: If ring buffer is full, it overwrites the
             * oldest data in ringbuffer.
             */
            ring_buffer_queue(&(ringBuf_rx[id]), rxChar);
        }
    }

    /* Clear Interrupt Flag */
    SCI_REG_PTR[id]->SCIFFRX.bit.RXFFOVRCLR = 1;
    SCI_REG_PTR[id]->SCIFFRX.bit.RXFFINTCLR = 1;
}


void UART_init(void)
{
    Error_Block eb;
    Hwi_Params hwiParams;
    UInt16 idx = 0;

    /* Guard against multiple initialization */
    if(!bInit) {
        Error_init(&eb);

        /*
         * Init ring buffer and SCI GPIO
         */
#if (CONFIG_ENABLE_UARTA)
        memset(uarta_buffer_tx, 0, sizeof(uarta_buffer_tx));
        memset(uarta_buffer_rx, 0, sizeof(uarta_buffer_rx));
        ring_buffer_init(&(ringBuf_tx[UART_A]), uarta_buffer_tx, sizeof(uarta_buffer_tx));
        ring_buffer_init(&(ringBuf_rx[UART_A]), uarta_buffer_rx, sizeof(uarta_buffer_rx));

        EALLOW;
        SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;
        UARTA_TX_CONFIG;
        UARTA_RX_CONFIG;
        EDIS;
#endif
#if (CONFIG_ENABLE_UARTB)
        memset(uartb_buffer_tx, 0, sizeof(uartb_buffer_tx));
        memset(uartb_buffer_rx, 0, sizeof(uartb_buffer_rx));
        ring_buffer_init(&(ringBuf_tx[UART_B]), uartb_buffer_tx, sizeof(uartb_buffer_tx));
        ring_buffer_init(&(ringBuf_rx[UART_B]), uartb_buffer_rx, sizeof(uartb_buffer_rx));

        EALLOW;
        SysCtrlRegs.PCLKCR0.bit.SCIBENCLK = 1;
        UARTB_TX_CONFIG;
        UARTB_RX_CONFIG;
        EDIS;
#endif
#if (CONFIG_ENABLE_UARTC)
        memset(uartc_buffer_tx, 0, sizeof(uartc_buffer_tx));
        memset(uartc_buffer_rx, 0, sizeof(uartc_buffer_rx));
        ring_buffer_init(&(ringBuf_tx[UART_C]), uartc_buffer_tx, sizeof(uartc_buffer_tx));
        ring_buffer_init(&(ringBuf_rx[UART_C]), uartc_buffer_rx, sizeof(uartc_buffer_rx));

        EALLOW;
        SysCtrlRegs.PCLKCR0.bit.SCICENCLK = 1;
        UARTC_TX_CONFIG;
        UARTC_RX_CONFIG;
        EDIS;
#endif
        for(idx = 0; idx < N_UART; idx++) {
            /*
             * HWI
             */
            Hwi_Params_init(&hwiParams);
            hwiParams.arg = idx;
            hwiParams.enableAck = true;
            hwiParams.instance->name = tag[idx];
            hwiHdl_uart_tx[idx] = Hwi_create(DEFAULT_HWI_INT_NUM[idx][0], uart_tx_hwi_handler, &hwiParams, &eb);
            Assert_isTrue((Error_check(&eb) == FALSE) && (hwiHdl_uart_tx[idx] != NULL), NULL);
            hwiHdl_uart_rx[idx] = Hwi_create(DEFAULT_HWI_INT_NUM[idx][1], uart_rx_hwi_handler, &hwiParams, &eb);
            Assert_isTrue((Error_check(&eb) == FALSE) && (hwiHdl_uart_rx[idx] != NULL), NULL);

            /*
             * SCI and FIFO Initalization
             */
            SCI_REG_PTR[idx]->SCICCR.all = 0x0007;
            SCI_REG_PTR[idx]->SCICTL1.all = 0x0003;
            SCI_REG_PTR[idx]->SCICTL2.bit.TXINTENA = 1;
            SCI_REG_PTR[idx]->SCICTL2.bit.RXBKINTENA = 1;
            SCI_REG_PTR[idx]->SCIHBAUD = (DEFAULT_UART_BRR[idx] >> 8) & 0x00FF;
            SCI_REG_PTR[idx]->SCILBAUD = DEFAULT_UART_BRR[idx] & 0x00FF;
            SCI_REG_PTR[idx]->SCIFFTX.all = 0xC000;
            SCI_REG_PTR[idx]->SCIFFRX.all = 0x0021;
            SCI_REG_PTR[idx]->SCIFFCT.all = 0x0000;
            /* Release from reset and Enable FIFO */
            SCI_REG_PTR[idx]->SCICTL1.bit.SWRESET = 1;
            SCI_REG_PTR[idx]->SCIFFTX.bit.TXFIFOXRESET = 1;
            SCI_REG_PTR[idx]->SCIFFRX.bit.RXFIFORESET = 1;
        }

        /*
         * Enable SCI interrupt
         */
#if(CONFIG_ENABLE_UARTA)
        PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
        PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
        IER |= M_INT9;
#endif
#if(CONFIG_ENABLE_UARTB)
        PieCtrlRegs.PIEIER9.bit.INTx3 = 1;
        PieCtrlRegs.PIEIER9.bit.INTx4 = 1;
        IER |= M_INT9;
#endif
#if(CONFIG_ENABLE_UARTC)
        PieCtrlRegs.PIEIER8.bit.INTx5 = 1;
        PieCtrlRegs.PIEIER8.bit.INTx6 = 1;
        IER |= M_INT8;
#endif

        bInit = true;
    }
}


Bool UART_init_done(void)
{
    return (bInit);
}


UInt16 UART_send(UART_ID_T uart_id, Char * pBuf, UInt16 count)
{
    UInt16 i;
    UInt16 j;
    UInt16 fifo_first = 0;
    UInt16 fifo_status;

    if((pBuf == NULL) || (count == 0) || (bInit == 0) || (uart_id >= N_UART)) {
        return 0;
    }

    /*
     * Disable Tx interrupt to prevent race condition while
     * setting up the buffer
     */
    SCI_REG_PTR[uart_id]->SCIFFTX.bit.TXFFIENA = 0;

    fifo_status = SCI_REG_PTR[uart_id]->SCIFFTX.bit.TXFFST;

    if((fifo_status == 0) &&
       (ring_buffer_is_empty(&(ringBuf_tx[uart_id])))) {
        /* fill uart fifo first before the ring buffer */
        fifo_first = 1;
        j = 0;
    }

    for(i = 0; i < count; i++) {
        if((fifo_first == 1) && (j < 16)) {
            /* Write to Tx FIFO */
            SCI_REG_PTR[uart_id]->SCITXBUF = pBuf[i];
            j++;
        } else {
            if(ring_buffer_is_full(&(ringBuf_tx[uart_id]))) {
                break;
            }
            ring_buffer_queue(&(ringBuf_tx[uart_id]), pBuf[i]);
        }
    }

    /* Reenable Tx interrupt */
    SCI_REG_PTR[uart_id]->SCIFFTX.bit.TXFFIENA = 1;

    return(i);
}


UInt16 UART_receive(UART_ID_T uart_id, Char * pBuf, UInt16 count)
{
    UInt16 i = 0;

    if((pBuf == NULL) || (count == 0) || ring_buffer_is_empty(&(ringBuf_rx[uart_id])) || (uart_id >= N_UART)) {
        return 0;
    }

    for(i = 0; (i < count) && (!ring_buffer_is_empty(&(ringBuf_rx[uart_id]))); i++) {
        ring_buffer_dequeue(&(ringBuf_rx[uart_id]), &(pBuf[i]));
    }

    return i;
}

