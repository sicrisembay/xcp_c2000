#ifndef XCP_CFG_H
#define XCP_CFG_H

#include "xdc/runtime/System.h"
/*----------------------------------------------------------------------------*/
/* Test */
/* Use XCP_PRINT to generate diagnostic messages */

//#define XCP_DISABLE_TESTMODE
#define XCP_ENABLE_TESTMODE

#ifdef XCP_ENABLE_TESTMODE
  /* Enable xcpPutchar */
  extern void XcpPutch(char ch);
  #define XCP_ENABLE_SERV_TEXT
  #define XCP_ENABLE_SERV_TEXT_PUTCHAR
//  #define ApplXcpSendStall() 0
  #define ApplXcpPrint(x, ...)  System_printf(x, ##__VA_ARGS__)
  #define XCP_PRINT(x, ...)     System_printf(x, ##__VA_ARGS__)
  #define XCP_ASSERT(x);
#else
  #define XCP_PRINT(x)
  #define XCP_ASSERT(x)
#endif


/*----------------------------------------------------------------------------*/
/* XCP parameters */

  /* 8-Bit qualifier */
typedef unsigned char  vuint8;
typedef signed char    vsint8;

/* 16-Bit qualifier */
typedef unsigned short vuint16;
typedef signed short   vsint16;

/* 32-Bit qualifier */
typedef unsigned long  vuint32;
typedef signed long    vsint32;


/*----------------------------------------------------------------------------*/
/* XCP protocol parameters */
#define XCP_ENABLE_USE_BYTE_ACCESS

/* Byte order */
//#define C_CPUTYPE_BIGENDIAN  /* Motorola */
#define C_CPUTYPE_LITTLEENDIAN /* Intel */

#define XCP_TI_C2000

/*
 * Memory Address Granularity (AG)
 * C2000 address granularity is 2 bytes
 */
//#define CPUMEM_AG_BYTE
#define CPUMEM_AG_WORD
//#define CPUMEM_AG_DWORD

/* XCP message length */
#define kXcpMaxCTO     8      /* Maximum CTO Message Lenght */
#define kXcpMaxDTO     8      /* Maximum DTO Message Lenght */

/* Enable/Disable parameter checking (save memory) */
//#define XCP_DISABLE_PARAMETER_CHECK
#define XCP_ENABLE_PARAMETER_CHECK

/* Enable COMM_MODE_INFO */
#define XCP_ENABLE_COMM_MODE_INFO

/* Enable GET_SEED and UNLOCK */
#define XCP_DISABLE_SEED_KEY

/* Enable xcpUserService() to handle user defined commands */
#define XCP_DISABLE_USER_COMMAND

/* Enable transmission of event messages */
#define XCP_DISABLE_SEND_EVENT

/* Disable FLASH programming */
/* Implement the flash programming feature in the ECU */
/* Not available in xcpBasic ! */
#define XCP_DISABLE_PROGRAM

/* Activate the flash programming kernel download support */
#define XCP_DISABLE_BOOTLOADER_DOWNLOAD


/*----------------------------------------------------------------------------*/
/* Disable/Enable Interrupts */

/* Has to be defined if XcpSendCallBack() can interrupt XcpEvent() */
/// TODO

/*----------------------------------------------------------------------------*/
/* Memory copy */

#define xcpMemCpy memcpy
#define xcpMemSet memset


/*----------------------------------------------------------------------------*/
/* XCP Calibration Parameters */

#define XCP_ENABLE_CALIBRATION

#define XCP_DISABLE_SHORT_DOWNLOAD
#define XCP_ENABLE_SHORT_UPLOAD

/* Enable block transfer */
#define XCP_DISABLE_BLOCK_UPLOAD
#define XCP_DISABLE_BLOCK_DOWNLOAD

/* Enable memory checksum */
/* The checksum will be calculated in XcpBackground() */
/* This may be implementation specific */
#define XCP_DISABLE_CHECKSUM
//#define kXcpChecksumMethod XCP_CHECKSUM_TYPE_ADD14

/* Enable Calibration Page switching and dynamic calibration overlay RAM allocation */
#define XCP_DISABLE_CALIBRATION_MEM_ACCESS_BY_APPL
#define XCP_DISABLE_CALIBRATION_PAGE
#define XCP_DISABLE_PAGE_COPY
#define XCP_DISABLE_SEGMENT_INFO
#define XCP_DISABLE_PAGE_INFO
#define XCP_DISABLE_PAGE_FREEZE


/*----------------------------------------------------------------------------*/
/* XCP Data Stimulation Parameters */

/* Synchronous Data Stimulation (STIM) */
#define XCP_DISABLE_STIM
//#define XCP_ENABLE_STIM
//#define kXcpStimOdtCount 16


/*----------------------------------------------------------------------------*/
/* XCP Data Acquisition Parameters */

/* Enable data acquisition */
#define XCP_ENABLE_DAQ

/* Memory space reserved for DAQ */
#define kXcpDaqMemSize 128

#define XCP_ENABLE_SEND_QUEUE
#define XCP_DISABLE_SEND_BUFFER
#define XCP_DISABLE_DAQ_PRESCALER
#define XCP_DISABLE_DAQ_OVERRUN_INDICATION
#define XCP_DISABLE_DAQ_RESUME
#define XCP_ENABLE_DAQ_PROCESSOR_INFO
#define XCP_ENABLE_DAQ_RESOLUTION_INFO
#define XCP_DISABLE_WRITE_DAQ_MULTIPLE
#define XCP_DISABLE_DAQ_HDR_ODT_DAQ

/* Enable DAQ Timestamps */
//#define XCP_DISABLE_DAQ_TIMESTAMP
#define XCP_ENABLE_DAQ_TIMESTAMP
#define kXcpDaqTimestampSize            DAQ_TIMESTAMP_WORD
#define kXcpDaqTimestampUnit            DAQ_TIMESTAMP_UNIT_1MS
#define kXcpDaqTimestampTicksPerUnit    1   // Timer0 is pre-scaled to 1us tick

/* Enable */
//#define XCP_DISABLE_DAQ_EVENT_INFO
#define XCP_ENABLE_DAQ_EVENT_INFO
#define kXcpMaxEvent                    2

#define XCP_ENABLE_NO_P2INT_CAST

#endif /* XCP_CFG_H */
