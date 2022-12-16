#include "XcpBasic.h"
#include "uart/uart.h"


#define XCP_PACKET_CHECKSUM
#define XCP_PACKET_CHECKSUM_PLUS_LEN
#define XCP_PACKET_SLIP


//------------------------------------------------------------------------------
// Transport Layer

#define SLIP_SYNC 0x9A
#define SLIP_ESC  0x9B

#if xcpMaxCTO>255
  #error
#endif
static vuint8 sBuf[kXcpMaxCTO+2]; // + Header + Checksum
static vuint8 sBufPtr = 0;
static vuint8 sync = 0;


vsint16 GetChar(void)
{
    vsint16 rxByte;
    vuint16 count;
    count = UART_receive(UART_A, (Char *)&rxByte, 1);
    if(count == 0) {
        return -1;
    } else {
        return rxByte;
    }
}


#ifdef XCP_PACKET_SLIP
vsint16 GetSlipChar( void )
{
    vuint8 b;
    vuint16 count;
    vuint16 timeout;

    count = UART_receive(UART_A, (Char *)&b, 1);
    if(count == 0) {
        return -1; // Empty
    }

    if (b == SLIP_SYNC) { // Sync
      return 0x100;
    }

    if (b == SLIP_ESC) { // Escape
        timeout = 0;
        do {
            count = UART_receive(UART_A, (Char *)&b, 1);
            if(count == 0) {
                if(++timeout > 1000) {
                    return -1; // Error Timeout
                }
            }
        } while(count == 0);

        switch (b) {
            case 0x00: {
                return SLIP_ESC;
            }
            case 0x01: {
                return SLIP_SYNC;
            }
            case SLIP_SYNC: {
                return 0x100;
            }
            default: {
                return -1; // Error
            }
        }
    }

    return (vsint16)b;
}
#else
#define GetSlipChar GetChar
#endif /* XCP_PACKET_SLIP */

static vuint8 xcpReceive(void)
{
    vsint16 b;
    vuint8 i;
    vuint8 l;
    vuint8 s;

    for(;;) {
        // SlipMode
        // A new packet always starts with a 0xFF
#ifdef XCP_PACKET_SLIP
        if (!sync) {
            // Get the sync byte
            b = GetChar();
            if (b < 0) {
                return 0;
            }
            if (b != SLIP_SYNC) {
                continue; // Skip this byte
            }
            sync = 1;
            sBufPtr = 0;
        }
#endif

        // Get the next byte
        b = GetSlipChar();
        if (b < 0) {
            return 0;
        }
#ifdef XCP_PACKET_SLIP
        if (b==0x100) { // Sync
            sync = 1; // restart
            sBufPtr = 0;
            continue;
        }
#endif

        // Save this byte
        sBuf[sBufPtr] = b;
        l = sBuf[0]; // sBuf[0] always is the packet length
        sBufPtr++;

        // Check if this is a valid packet
        if (l==0) {
            sync = 0;
            sBufPtr = 0;
            continue;
        }

#ifdef XCP_PACKET_CHECKSUM
        // Check if the packet is complete
        if (sBufPtr <= l+1) {
            continue;
        }

        // Verify checksum
#ifdef XCP_PACKET_CHECKSUM_PLUS_LEN
        s = l;
#else
        s = 0;
#endif
        for (i = 0; i < l; i++) {
            s += sBuf[i+1];
        }
        if (s != sBuf[sBufPtr-1]) { // Checksum error
            sync = 0;
            sBufPtr = 0;
            return 0;
        }
#else
        // Check if the packet is complete
        if (sBufPtr <= l) {
            continue;
        }
#endif

        // Return the packet
        sync = 0;
        sBufPtr = 0;
        return 1;
    } // for(;;)
}


void XcpReceiveCommand(void)
{
    if (xcpReceive()) {
        XcpCommand( (void *)&sBuf[1] );
    }
}


#ifdef XCP_PACKET_SLIP
vuint8 TransmitSlipByte( vuint8 b )
{
    vuint8 temp = 0;
    vuint8 ret = 0;

    switch (b) {
        case SLIP_ESC: {
            temp = SLIP_ESC;
            ret = (vuint8)(UART_send(UART_A, (Char *)&temp, 1));
            temp = 0x00;
            ret = ret && (vuint8)(UART_send(UART_A, (Char *)&temp, 1));
        }
        case SLIP_SYNC: {
            temp = SLIP_ESC;
            ret = (vuint8)(UART_send(UART_A, (Char *)&temp, 1));
            temp = 0x01;
            ret = ret && (vuint8)(UART_send(UART_A, (Char *)&temp, 1));
        }
        default: {
            ret = (vuint8)(UART_send(UART_A, (Char *)&b, 1));
        }
    }

    return (ret);
}
#else
vuint8 TransmitSlipByte( vuint8 b )
{
    return((vuint8)(UART_send(UART_A, (Char *)&b, 1)));
}
#endif


// Transmit a XCP packet
// Called by the XCP driver
void ApplXcpSend( vuint8 len, MEMORY_ROM BYTEPTR msg1 )
{
    vuint8 b;
    vuint8 temp = 0;

    BYTEPTR msg = (BYTEPTR)msg1;
#ifdef XCP_PACKET_CHECKSUM
    vuint8 checksum;
#endif

    if ((len == 0) || (len > kXcpMaxDTO)) {
        return; // should not happen
    }

#ifdef XCP_PACKET_SLIP
    temp = SLIP_SYNC;
    if (!UART_send(UART_A, (Char *)&temp, 1)) {
        return; // Each packet begins with 0xFF
    }
#endif
    if (!TransmitSlipByte(len)) {
        return;
    }

#ifdef XCP_PACKET_CHECKSUM
#ifdef XCP_PACKET_CHECKSUM_PLUS_LEN
    checksum = len;
#else
    checksum = 0;
#endif // XCP_PACKET_CHECKSUM_PLUS_LEN
#endif // XCP_PACKET_CHECKSUM

    while (len--) {
        b = *msg++;
#ifdef XCP_PACKET_CHECKSUM
        checksum += b;
#endif // XCP_PACKET_CHECKSUM
        if (!TransmitSlipByte(b)) {
            return;
        }
    }

#ifdef XCP_PACKET_CHECKSUM
    if (!TransmitSlipByte(checksum)) {
        return;
    }
#endif

  return;
}


// Convert a XCP address to a pointer
MTABYTEPTR ApplXcpGetPointer( vuint8 addr_ext, vuint32 addr )
{
    addr_ext = addr_ext;
    return (MTABYTEPTR)addr;
}

