#ifndef DRIVER_UART_H
#define DRIVER_UART_H

#include "driver_def.h"
#include <xdc/std.h>

#if CONFIG_USE_UART || __DOXYGEN__

typedef enum {
#if CONFIG_ENABLE_UARTA
    UART_A,                 /*!< ID for UART-A */
#endif
#if CONFIG_ENABLE_UARTB
    UART_B,                 /*!< ID for UART-B */
#endif
#if CONFIG_ENABLE_UARTC
    UART_C,                 /*!< ID for UART-C */
#endif
    N_UART                  /*!< Number of UART instance (always last in enumeration) */
} UART_ID_T;


void UART_init(void);


Bool UART_init_done(void);


UInt16 UART_send(UART_ID_T uart_id, Char * pBuf, UInt16 count);


UInt16 UART_receive(UART_ID_T uart_id, Char * pBuf, UInt16 count);

#endif /* CONFIG_USE_UART || __DOXYGEN__ */
#endif /* DRIVER_UART_H */
