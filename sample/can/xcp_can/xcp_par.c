#include "xcpBasic.h"

#if defined(kXcpMaxEvent)
vuint8 const kXcpEventDirection[kXcpMaxEvent] = {
        DAQ_EVENT_DIRECTION_DAQ,
        DAQ_EVENT_DIRECTION_DAQ
};

vuint8 const kXcpEventCycle[kXcpMaxEvent] = {
        10,  /* 10ms */
        100  /* 100ms */
};

vuint8 const kXcpEventUnit[kXcpMaxEvent] = {
        DAQ_TIMESTAMP_UNIT_1MS,
        DAQ_TIMESTAMP_UNIT_1MS
};

char * const kXcpEventName[kXcpMaxEvent] = {
        "10ms",
        "100ms"
};

vuint8 const kXcpEventNameLength[kXcpMaxEvent] = {
        4, /* "10ms" */
        5, /* "100ms" */
};

#endif
