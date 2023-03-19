/*
 *  ======== main.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include "f2833x/v142/DSP2833x_headers/include/DSP2833x_Device.h"
#include "XcpBasic.h"
#include "driver_def.h"
#include <math.h>


/*
 *  ======== taskFxn ========
 */

#define PI                   3.14159265
#define TWO_PI               2.0f * PI
#define PHASEB               TWO_PI / 3.0f
#define PHASEC               4.0f * PI / 3.0f

float waveformA;
float waveformB;
float waveformC;
float amplitudeA;
float amplitudeB;
float amplitudeC;
float freq;
UInt16 testTick;

Void taskFxn(UArg a0, UArg a1)
{
    float t = 0.0;
    float period = 1.0f;
    extern void XcpHandler(void);
    uint16_t prescaler_10ms = 0;
    uint16_t prescaler_100ms = 0;
    uint16_t blink_counter = 0;
    amplitudeA = 1.0f;
    amplitudeB = 1.0f;
    amplitudeC = 1.0f;
    freq = 1.0f;  // 1Hz
    testTick = 0;

    waveformA = amplitudeA * sin(TWO_PI * freq * t);
    waveformB = amplitudeB * sin((TWO_PI * freq * t) + PHASEB);
    waveformC = amplitudeA * sin((TWO_PI * freq * t) + PHASEC);

    extern void XcpCanInit(void);

    XcpCanInit();

    XcpInit();

    EALLOW;
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 1;
    EDIS;
    GpioDataRegs.GPBSET.bit.GPIO34 = 1; // LED off

    System_printf("running taskFxn()\n");

    while(1) {
        /* XcpHandler at 1ms interval */
        Task_sleep(1);
        if(freq != 0.0f) {
            period = 1/freq;
        }
        t += 0.001;
        while(t > period) {
            t = t - period;
        }

        waveformA = amplitudeA * sin(TWO_PI * freq * t);
        waveformB = amplitudeB * sin((TWO_PI * freq * t) + PHASEB);
        waveformC = amplitudeC * sin((TWO_PI * freq * t) + PHASEC);

        XcpHandler();

        prescaler_10ms++;
        prescaler_100ms++;
        blink_counter++;

        if(prescaler_10ms >= 10) {
            /* 10ms event */
            XcpEvent(XCP_EVENT_10MS);
            prescaler_10ms = 0;
            testTick++;
        }

        if(prescaler_100ms >= 100) {
            /* 100ms event */
            XcpEvent(XCP_EVENT_100MS);
            prescaler_100ms = 0;
        }

        if(blink_counter < 200) {
            GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
        } else if(blink_counter < 2000) {
            GpioDataRegs.GPBSET.bit.GPIO34 = 1;
        } else {
            blink_counter = 0;
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


#pragma CODE_SECTION(InitFlashWaitState, "ramfuncs");
static void InitFlashWaitState(void)
{
    EALLOW;

    //
    // Enable Flash Pipeline mode to improve performance
    // of code executed from Flash.
    //
    FlashRegs.FOPT.bit.ENPIPE = 1;

    //
    //                CAUTION
    // Minimum waitstates required for the flash operating
    // at a given CPU rate must be characterized by TI.
    // Refer to the datasheet for the latest information.
    //
    //
    // Set the Paged Waitstate for the Flash
    //
    FlashRegs.FBANKWAIT.bit.PAGEWAIT = 5;

    //
    // Set the Random Waitstate for the Flash
    //
    FlashRegs.FBANKWAIT.bit.RANDWAIT = 5;

    //
    // Set the Waitstate for the OTP
    //
    FlashRegs.FOTPWAIT.bit.OTPWAIT = 8;

    //
    //                CAUTION
    // ONLY THE DEFAULT VALUE FOR THESE 2 REGISTERS SHOULD BE USED
    //
    FlashRegs.FSTDBYWAIT.bit.STDBYWAIT = 0x01FF;
    FlashRegs.FACTIVEWAIT.bit.ACTIVEWAIT = 0x01FF;

    EDIS;

    //
    // Force a pipeline flush to ensure that the write to
    // the last register configured occurs before returning.
    //
    asm(" RPT #7 || NOP");
}


static void configure_core_pll(UInt16 val)
{
    /* Make sure the PLL is not running in limp mode */
    if (SysCtrlRegs.PLLSTS.bit.MCLKSTS != 0)
    {
       /*
        * Missing external clock has been detected
        * Replace this line with a call to an appropriate
        * SystemShutdown(); function.
        */
       asm("        ESTOP0");
    }

    /* Change the PLLCR */
    if (SysCtrlRegs.PLLCR.bit.DIV != val)
    {

       EALLOW;
       /* Before setting PLLCR turn off missing clock detect logic */
       SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;
       SysCtrlRegs.PLLCR.bit.DIV = val;
       EDIS;

       /*
        * Wait for PLL to lock.
        * During this time the CPU will switch to OSCCLK/2 until
        * the PLL is stable.  Once the PLL is stable the CPU will
        *
        * Wait for the PLL lock bit to be set.
        */
       while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS == 0) {
           /*
            * Note: The watchdog should be fed within
            * the loop via ServiceDog().
            */
       }

       EALLOW;
       SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0;
       EDIS;
     }
}


void CLOCK_init(void)
{
    Types_FreqHz coreFreq;

    /* Configure Core Frequency */
    DINT;
    IER = 0x0000;
    IFR = 0x0000;

    configure_core_pll(0xA);

    InitFlashWaitState();

    /* Update SYSBIOS Core Clock */
    coreFreq.hi = 0;
    coreFreq.lo = CONFIG_SYSTEM_FREQ_MHZ * 1000000U;
    BIOS_setCpuFreq(&coreFreq);
    Clock_tickReconfig();
}


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

    CLOCK_init();

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
