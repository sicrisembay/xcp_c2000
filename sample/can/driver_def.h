#ifndef DRIVER_DEF_H
#define DRIVER_DEF_H

#define CONFIG_SYSTEM_FREQ_MHZ  150

/*
 * XCP CAN
 */
#define CONFIG_XCP_TX_CAN_ID            0x554
#define CONFIG_XCP_RX_CAN_ID            0x555
#define CONFIG_XCP_CAN_TX_MB            0
#define CONFIG_XCP_CAN_RX_MB            16

/*
 * UART Driver Configuration
 */
#define CONFIG_USE_UART                 1
#define CONFIG_ENABLE_UARTA             1
#if CONFIG_ENABLE_UARTA
#define CONFIG_BAUDRATE_UARTA           115200
#define CONFIG_TX_QUEUE_BUFF_SZ_UARTA   1024
#define CONFIG_RX_QUEUE_BUFF_SZ_UARTA   128
#endif

#define CONFIG_ENABLE_UARTB             0
#if CONFIG_ENABLE_UARTB
#define CONFIG_BAUDRATE_UARTB           115200
#define CONFIG_TX_QUEUE_BUFF_SZ_UARTB   256
#define CONFIG_RX_QUEUE_BUFF_SZ_UARTB   128
#endif

#define CONFIG_ENABLE_UARTC             0
#if CONFIG_ENABLE_UARTC
#define CONFIG_BAUDRATE_UARTC           115200
#define CONFIG_TX_QUEUE_BUFF_SZ_UARTC   256
#define CONFIG_RX_QUEUE_BUFF_SZ_UARTC   128
#endif

/*
 * CAN Driver Configurtion
 */
#define CONFIG_USE_CAN                  1
#define CONFIG_ENABLE_CAN_A             1
#if CONFIG_ENABLE_CAN_A
#define CONFIG_CAN_A_BPS_250KHZ         0
#define CONFIG_CAN_A_BPS_1MHZ           1
#define CONFIG_CAN_A_DBO_LSB            1
#if CONFIG_CAN_A_BPS_1MHZ
#define CONFIG_CAN_A_BRP                4
#define CONFIG_CAN_A_TSEG1              10  /* Fclk 150MHz */
#define CONFIG_CAN_A_TSEG2              2   /* Fclk 150MHz */
#elif CONFIG_CAN_A_BPS_250KHZ
#define CONFIG_CAN_A_BRP                19
#define CONFIG_CAN_A_TSEG1              10  /* Fclk 150MHz */
#define CONFIG_CAN_A_TSEG2              2   /* Fclk 150MHz */
#endif
#define CONFIG_CAN_A_TX_GPIO31          1
#define CONFIG_CAN_A_RX_GPIO30          1
#endif

#define CONFIG_ENABLE_CAN_B             0

#endif /* DRIVER_DEF_H */
