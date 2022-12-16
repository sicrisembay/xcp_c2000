/*
 *  ======== main.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include "uart/uart.h"
#include "XcpBasic.h"

/*
 *  ======== taskFxn ========
 */
Void taskFxn(UArg a0, UArg a1)
{
    extern void XcpReceiveCommand(void);
    uint16_t i = 0;

    UART_init();

    XcpInit();

    while(1) {
        /* 10ms Polling */
        Task_sleep(10);
        XcpReceiveCommand();
        XcpEvent(1);

        i++;
        if(i >= 10) {
            /* 100ms Polling */
            i = 0;
            XcpEvent(2);
        }
    }
}


/*
 *  ======== main ========
 */
extern unsigned int RamfuncsLoadStart;
extern unsigned int RamfuncsLoadEnd;
extern unsigned int RamfuncsRunStart;
extern unsigned int RamConstLoadStart;
extern unsigned int RamConstLoadEnd;
extern unsigned int RamConstRunStart;

Int main()
{ 
    Task_Handle task;
    Task_Params tskParams;
    Error_Block eb;

    /*
     * Copy ramfuncs section
     */
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart,
           &RamfuncsLoadEnd - &RamfuncsLoadStart);

    /*
     * Copy ramconsts section
     */
    memcpy(&RamConstRunStart, &RamConstLoadStart,
           &RamConstLoadEnd - &RamConstLoadStart);

    System_printf("enter main()\n");

    Error_init(&eb);
    Task_Params_init(&tskParams);
    tskParams.priority = 1;
    tskParams.stackSize = 512;
    task = Task_create(taskFxn, &tskParams, &eb);
    if (task == NULL) {
        System_printf("Task_create() failed!\n");
        BIOS_exit(0);
    }

    BIOS_start();    /* does not return */
    return(0);
}
