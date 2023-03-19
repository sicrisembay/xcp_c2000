#ifndef XCP_PAR_H
#define XCP_PAR_H

#include "xcp_cfg.h"

/* declare here parameters for customizing XcpBasic driver (e.g. kXcpStationId) */

#if defined(kXcpMaxEvent)

typedef enum {
    XCP_EVENT_10MS = 0,
    XCP_EVENT_100MS,

    MAX_XCP_EVENT
} XCP_EVENT_T;

extern vuint8 const kXcpEventDirection[kXcpMaxEvent];
extern vuint8 const kXcpEventCycle[kXcpMaxEvent];
extern vuint8 const kXcpEventUnit[kXcpMaxEvent];
extern vuint8 const kXcpEventNameLength[kXcpMaxEvent];
extern char * const kXcpEventName[kXcpMaxEvent];
extern vuint8 const kXcpEventNameLength[kXcpMaxEvent];
#endif


#endif /* XCP_PAR_H */
