MEMORY
{
PAGE 0:    /* Program Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE1 for data allocation */

    RAML0_2     : origin = 0x008000, length = 0x003000     /* on-chip RAM block L0 L1 L2 used for RAM functions */
    FLASHRSVD   : origin = 0x300000, length = 0x008000     /* RESERVED FLASH */
    FLASHFWUPDATE : origin = 0x308000, length = 0x018000   /* FLASH AREA FOR FIRMWARE UPDATE */
    FLASHAPP    : origin = 0x320000 , length = 0x017C00    /* FLASH AREA FOR FIRMWARE EXECUTION */
    BEGIN       : origin = 0x337C00, length = 0x000008     /* APP START. CLEARCUT BOOTLOADER WILL JUMP HERE */
    FLASHAPP_HDR : origin = 0x337C08, length = 0x0003F8    /* Header */
    FLASHBOOT   : origin = 0x338000, length = 0x007F80     /* FLASH AREA FOR BOOT */
    CSM_RSVD    : origin = 0x33FF80, length = 0x000076     /* Part of BOOT.  Program with all 0x0000 when CSM is in use. */
    BOOT_BEGIN  : origin = 0x33FFF6, length = 0x000002     /* Part of BOOT.  Used for "boot to Flash" bootloader mode. */
    CSM_PWL     : origin = 0x33FFF8, length = 0x000008     /* Part of FLASHA.  CSM password locations in FLASHA */

    OTP         : origin = 0x380400, length = 0x000400     /* on-chip OTP */
    ADC_CAL     : origin = 0x380080, length = 0x000009     /* ADC_cal function in Reserved memory */

    IQTABLES    : origin = 0x3FE000, length = 0x000b50     /* IQ Math Tables in Boot ROM */
    IQTABLES2   : origin = 0x3FEB50, length = 0x00008c     /* IQ Math Tables in Boot ROM */
    FPUTABLES   : origin = 0x3FEBDC, length = 0x0006A0     /* FPU Tables in Boot ROM */
    ROM         : origin = 0x3FF27C, length = 0x000D44     /* Boot ROM */
    RESET       : origin = 0x3FFFC0, length = 0x000002     /* part of boot ROM  */
    VECTORS     : origin = 0x3FFFC2, length = 0x00003E     /* part of boot ROM  */

PAGE 1 :   /* Data Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE0 for program allocation */
           /* Registers remain on PAGE1                                                  */

    BOOT_RSVD   : origin = 0x000000, length = 0x000040     /* Part of M0, BOOT rom will use this for stack */
    RAMM0       : origin = 0x000040, length = 0x0003C0     /* on-chip RAM block M0 */
    RAMM1       : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
    RAML3_6     : origin = 0x00B000, length = 0x004000     /* on-chip RAM block L3 - L6 */
    RAML7       : origin = 0x00F000, length = 0x001000     /* on-chip RAM block L7 */

    DEV_EMU     : origin = 0x000880, length = 0x000180     /* device emulation registers */
    FLASH_REGS  : origin = 0x000A80, length = 0x000060     /* FLASH registers */
    CSM         : origin = 0x000AE0, length = 0x000010     /* code security module registers */
    ADC_MIRROR  : origin = 0x000B00, length = 0x000010     /* ADC Results register mirror */
    XINTF       : origin = 0x000B20, length = 0x000020     /* external interface registers */
    CPU_TIMER0  : origin = 0x000C00, length = 0x000008     /* CPU Timer0 registers */
    CPU_TIMER1  : origin = 0x000C08, length = 0x000008     /* CPU Timer0 registers (CPU Timer1 & Timer2 reserved TI use)*/
    CPU_TIMER2  : origin = 0x000C10, length = 0x000008     /* CPU Timer0 registers (CPU Timer1 & Timer2 reserved TI use)*/
    PIE_CTRL    : origin = 0x000CE0, length = 0x000020     /* PIE control registers */
    PIE_VECT    : origin = 0x000D00, length = 0x000100     /* PIE Vector Table */
    DMA         : origin = 0x001000, length = 0x000200     /* DMA registers */
    MCBSPA      : origin = 0x005000, length = 0x000040     /* McBSP-A registers */
    MCBSPB      : origin = 0x005040, length = 0x000040     /* McBSP-B registers */
    ECANA       : origin = 0x006000, length = 0x000040     /* eCAN-A control and status registers */
    ECANA_LAM   : origin = 0x006040, length = 0x000040     /* eCAN-A local acceptance masks */
    ECANA_MOTS  : origin = 0x006080, length = 0x000040     /* eCAN-A message object time stamps */
    ECANA_MOTO  : origin = 0x0060C0, length = 0x000040     /* eCAN-A object time-out registers */
    ECANA_MBOX  : origin = 0x006100, length = 0x000100     /* eCAN-A mailboxes */
    ECANB       : origin = 0x006200, length = 0x000040     /* eCAN-B control and status registers */
    ECANB_LAM   : origin = 0x006240, length = 0x000040     /* eCAN-B local acceptance masks */
    ECANB_MOTS  : origin = 0x006280, length = 0x000040     /* eCAN-B message object time stamps */
    ECANB_MOTO  : origin = 0x0062C0, length = 0x000040     /* eCAN-B object time-out registers */
    ECANB_MBOX  : origin = 0x006300, length = 0x000100     /* eCAN-B mailboxes */
    EPWM1       : origin = 0x006800, length = 0x000022     /* Enhanced PWM 1 registers */
    EPWM2       : origin = 0x006840, length = 0x000022     /* Enhanced PWM 2 registers */
    EPWM3       : origin = 0x006880, length = 0x000022     /* Enhanced PWM 3 registers */
    EPWM4       : origin = 0x0068C0, length = 0x000022     /* Enhanced PWM 4 registers */
    EPWM5       : origin = 0x006900, length = 0x000022     /* Enhanced PWM 5 registers */
    EPWM6       : origin = 0x006940, length = 0x000022     /* Enhanced PWM 6 registers */
    ECAP1       : origin = 0x006A00, length = 0x000020     /* Enhanced Capture 1 registers */
    ECAP2       : origin = 0x006A20, length = 0x000020     /* Enhanced Capture 2 registers */
    ECAP3       : origin = 0x006A40, length = 0x000020     /* Enhanced Capture 3 registers */
    ECAP4       : origin = 0x006A60, length = 0x000020     /* Enhanced Capture 4 registers */
    ECAP5       : origin = 0x006A80, length = 0x000020     /* Enhanced Capture 5 registers */
    ECAP6       : origin = 0x006AA0, length = 0x000020     /* Enhanced Capture 6 registers */
    EQEP1       : origin = 0x006B00, length = 0x000040     /* Enhanced QEP 1 registers */
    EQEP2       : origin = 0x006B40, length = 0x000040     /* Enhanced QEP 2 registers */
    GPIOCTRL    : origin = 0x006F80, length = 0x000040     /* GPIO control registers */
    GPIODAT     : origin = 0x006FC0, length = 0x000020     /* GPIO data registers */
    GPIOINT     : origin = 0x006FE0, length = 0x000020     /* GPIO interrupt/LPM registers */
    SYSTEM      : origin = 0x007010, length = 0x000020     /* System control registers */
    SPIA        : origin = 0x007040, length = 0x000010     /* SPI-A registers */
    SCIA        : origin = 0x007050, length = 0x000010     /* SCI-A registers */
    XINTRUPT    : origin = 0x007070, length = 0x000010     /* external interrupt registers */
    ADC         : origin = 0x007100, length = 0x000020     /* ADC registers */
    SCIB        : origin = 0x007750, length = 0x000010     /* SCI-B registers */
    SCIC        : origin = 0x007770, length = 0x000010     /* SCI-C registers */
    I2CA        : origin = 0x007900, length = 0x000040     /* I2C-A registers */
    CSM_PWL     : origin = 0x33FFF8, length = 0x000008     /* Part of FLASHA.  CSM password locations. */
    PARTID      : origin = 0x380090, length = 0x000001     /* Part ID register location */
}

/*
   Allocate sections to memory blocks.
   Note:
         boot_codestart :  This is a entry point of bootloader code.  This is fetched
                           and executed by BOOT ROM.
         app_codestart  :  This is an entry point of application code.  This is fetched
                           and executed by bootloader code (refer to boot.asm)

         Boot sequence:

         [Power-ON] --> BOOT ROM executes ---(jumps to boot_codestart)--> Bootloader executes --
               --(jumps app_codestart)--> application firmware executes
*/

SECTIONS
{
   /* Allocate program areas: */
    .cinit              : > FLASHAPP      PAGE = 0
    .pinit              : > FLASHAPP      PAGE = 0
    .text               : > FLASHAPP      PAGE = 0
    app_codestart       : > BEGIN         PAGE = 0
    boot_codestart      : > BOOT_BEGIN    PAGE = 0
    copysections        : > FLASHAPP      PAGE = 0
    ramfuncs            : LOAD = FLASHAPP PAGE = 0,
                          RUN = RAML0_2   PAGE = 0,
                          LOAD_START(_RamfuncsLoadStart),
                          LOAD_SIZE(_RamfuncsLoadSize),
                          LOAD_END(_RamfuncsLoadEnd),
                          RUN_START(_RamfuncsRunStart)
    ramconsts           : LOAD = FLASHAPP PAGE = 0,
                          RUN = RAML3_6   PAGE = 1,
                          LOAD_START(_RamConstLoadStart),
                          LOAD_SIZE(_RamConstLoadSize),
                          LOAD_END(_RamConstLoadEnd),
                          RUN_START(_RamConstRunStart)

    csmpasswds          : > CSM_PWL     PAGE = 0
    csm_rsvd            : > CSM_RSVD    PAGE = 0

    /* Allocate uninitalized data sections: */
    .stack              : > RAMM1              PAGE = 1
    .ebss               : > RAMM1 | RAML3_6    PAGE = 1
    .data               : > RAMM1 | RAML3_6    PAGE = 1
    .esysmem            : > RAMM1 | RAML3_6    PAGE = 1
    .cio                : > RAMM1 | RAML3_6    PAGE = 1

    /* Initalized sections go in Flash */
    /* For SDFlash to program these, they must be allocated to page 0 */
    .econst             : > FLASHAPP      PAGE = 0
    .switch             : > FLASHAPP      PAGE = 0

    /* Allocate IQ math areas: */
    IQmath              : > FLASHAPP      PAGE = 0                  /* Math Code */
    IQmathTables        : > IQTABLES,  PAGE = 0, TYPE = NOLOAD

    /* Uncomment the section below if calling the IQNexp() or IQexp()
       functions from the IQMath.lib library in order to utilize the
       relevant IQ Math table in Boot ROM (This saves space and Boot ROM
       is 1 wait-state). If this section is not uncommented, IQmathTables2
       will be loaded into other memory (SARAM, Flash, etc.) and will take
       up space, but 0 wait-state is possible.
    */
    /*
    IQmathTables2    : > IQTABLES2, PAGE = 0, TYPE = NOLOAD
    {
               IQmath.lib<IQNexpTable.obj> (IQmathTablesRam)
    }
    */

    FPUmathTables    : > FPUTABLES, PAGE = 0, TYPE = NOLOAD

    /* .reset is a standard section used by the compiler.  It contains the */
    /* the address of the start of _c_int00 for C Code.   /*
    /* When using the boot ROM this section and the CPU vector */
    /* table is not needed.  Thus the default type is set here to  */
    /* DSECT  */
    .reset           : > RESET,       PAGE = 0, TYPE = DSECT
    vectors          : > VECTORS      PAGE = 0, TYPE = DSECT

    /* Allocate ADC_cal function (pre-programmed by factory into TI reserved memory) */
    .adc_cal         : load = ADC_CAL,   PAGE = 0, TYPE = NOLOAD

    DLOG             : > RAML3_6,     PAGE = 1

    DMARAM           : > RAML7,       PAGE = 1


    PieVectTableFile : > PIE_VECT,   PAGE = 1
    /*** Peripheral Frame 0 Register Structures ***/
    DevEmuRegsFile    : > DEV_EMU,     PAGE = 1
    FlashRegsFile     : > FLASH_REGS,  PAGE = 1
    CsmRegsFile       : > CSM,         PAGE = 1
    AdcMirrorFile     : > ADC_MIRROR,  PAGE = 1
    XintfRegsFile     : > XINTF,       PAGE = 1
    CpuTimer0RegsFile : > CPU_TIMER0,  PAGE = 1
    CpuTimer1RegsFile : > CPU_TIMER1,  PAGE = 1
    CpuTimer2RegsFile : > CPU_TIMER2,  PAGE = 1
    PieCtrlRegsFile   : > PIE_CTRL,    PAGE = 1
    DmaRegsFile       : > DMA,         PAGE = 1
    /*** Peripheral Frame 3 Register Structures ***/
    McbspaRegsFile    : > MCBSPA,      PAGE = 1
    McbspbRegsFile    : > MCBSPB,      PAGE = 1
    /*** Peripheral Frame 1 Register Structures ***/
    ECanaRegsFile     : > ECANA,       PAGE = 1
    ECanaLAMRegsFile  : > ECANA_LAM    PAGE = 1
    ECanaMboxesFile   : > ECANA_MBOX   PAGE = 1
    ECanaMOTSRegsFile : > ECANA_MOTS   PAGE = 1
    ECanaMOTORegsFile : > ECANA_MOTO   PAGE = 1
    ECanbRegsFile     : > ECANB,       PAGE = 1
    ECanbLAMRegsFile  : > ECANB_LAM    PAGE = 1
    ECanbMboxesFile   : > ECANB_MBOX   PAGE = 1
    ECanbMOTSRegsFile : > ECANB_MOTS   PAGE = 1
    ECanbMOTORegsFile : > ECANB_MOTO   PAGE = 1
    EPwm1RegsFile     : > EPWM1        PAGE = 1
    EPwm2RegsFile     : > EPWM2        PAGE = 1
    EPwm3RegsFile     : > EPWM3        PAGE = 1
    EPwm4RegsFile     : > EPWM4        PAGE = 1
    EPwm5RegsFile     : > EPWM5        PAGE = 1
    EPwm6RegsFile     : > EPWM6        PAGE = 1
    ECap1RegsFile     : > ECAP1        PAGE = 1
    ECap2RegsFile     : > ECAP2        PAGE = 1
    ECap3RegsFile     : > ECAP3        PAGE = 1
    ECap4RegsFile     : > ECAP4        PAGE = 1
    ECap5RegsFile     : > ECAP5        PAGE = 1
    ECap6RegsFile     : > ECAP6        PAGE = 1
    EQep1RegsFile     : > EQEP1        PAGE = 1
    EQep2RegsFile     : > EQEP2        PAGE = 1
    GpioCtrlRegsFile  : > GPIOCTRL     PAGE = 1
    GpioDataRegsFile  : > GPIODAT      PAGE = 1
    GpioIntRegsFile   : > GPIOINT      PAGE = 1
    /*** Peripheral Frame 2 Register Structures ***/
    SysCtrlRegsFile   : > SYSTEM,      PAGE = 1
    SpiaRegsFile      : > SPIA,        PAGE = 1
    SciaRegsFile      : > SCIA,        PAGE = 1
    XIntruptRegsFile  : > XINTRUPT,    PAGE = 1
    AdcRegsFile       : > ADC,         PAGE = 1
    ScibRegsFile      : > SCIB,        PAGE = 1
    ScicRegsFile      : > SCIC,        PAGE = 1
    I2caRegsFile      : > I2CA,        PAGE = 1
    /*** Code Security Module Register Structures ***/
    CsmPwlFile        : > CSM_PWL,     PAGE = 1
    /*** Device Part ID Register Structures ***/
    PartIdRegsFile    : > PARTID,      PAGE = 1
}



