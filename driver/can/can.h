/*!
 * \file can.h
 */

#ifndef COMM_CAN_H
#define COMM_CAN_H

#include "driver_def.h"

#if CONFIG_USE_CAN || __DOXYGEN__

#include <ti/sysbios/knl/Mailbox.h>

/*!
 * \enum CAN_PERIPH_ID_T
 *
 * Enumeration of CAN Peripheral ID
 */
typedef enum {
#if (CONFIG_ENABLE_CAN_A)
    CAN_A,              /*!< ID for first instance of CAN peripheral */
#endif /* CONFIG_ENABLE_CAN_A */
#if (CONFIG_ENABLE_CAN_B)
    CAN_B,              /*!< ID for second instance of CAN peripheral */
#endif /* (CONFIG_ENABLE_CAN_B) */
    N_USE_CAN           /*!< Number of CAN instances (always last in enumeration) */
} CAN_PERIPH_ID_T;


/*!
 * \enum CAN_MAILBOX_EVT_T
 *
 * Enumeration of CAN Mailbox Event ID
 */
typedef enum {
    CAN_MAILBOX_RECEIVE_EVT = 0,
    CAN_MAILBOX_TRANSMIT_DONE_EVT,
    CAN_MAILBOX_ERROR,

    N_CAN_MAILBOX_EVT
} CAN_MAILBOX_EVT_T;


typedef struct {
    UInt16 msgId;
    UInt16 mailboxNumber;
    CAN_MAILBOX_EVT_T evtId;
    UInt16 len;
    UInt8 rtr;
    UInt8 data[8];
} CAN_MAILBOX_ENTRY_T;

/*!
 * \page page_can
 * \section section_can_intf CAN Interface
 */


/*!
 * \page page_can
 * \subsection subsect_can_init CAN_init
 * <PRE>void CAN_init(void);</PRE>
 *
 * \fn void CAN_init(void)
 *
 * This function initializes Controller Area Network (CAN).
 *
 * \param None
 *
 * \return None
 */
void CAN_init(void);


/*!
 * \page page_can
 * \subsection subsect_can_init_done CAN_init_done
 * <PRE>Bool CAN_init_done(void);</PRE>
 *
 * \fn Bool CAN_init_done(void)
 *
 * This function returns the initialization state CAN peripheral.
 *
 * \param None
 *
 * \return true: init done; false: otherwise
 */
Bool CAN_init_done(void);


/*!
 * \page page_can
 * \subsection subsect_can_rx_config CAN_rx_config
 * <PRE>Bool CAN_rx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 msgID,
 *                      Bool bIsTx, Bool overwriteProtect, Mailbox_Handle pMB);</PRE>
 *
 * \fn Bool CAN_rx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 msgIdFilter, UInt16 filterMask, Bool overwriteProtect, Mailbox_Handle pMB)
 *
 * This function configures a specified mailbox of a specified CAN peripheral.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     Mailbox number. Valid value is 0-31, inclusive
 * \param msgIdFilter       11-bit Message ID assigned to this mailbox
 * \param filterMask        11-bit Acceptance (bit-wise) filter for incoming message ID. Set bit is don't care.
 * \param overwriteProtect  Mailbox overwrite protection. Set to true to enable protection.
 * \param pMB               SysBIOS mailbox handle for event and data notification
 *
 * \return true: successful; false: otherwise
 */
Bool CAN_rx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 msgIdFilter, UInt16 filterMask, Bool overwriteProtect, Mailbox_Handle pMB);


/*!
 * \page page_can
 * \subsection subsect_can_tx_config CAN_tx_config
 * <PRE>Bool CAN_tx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 mailboxSize);</PRE>
 *
 * \fn Bool CAN_tx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 mailboxSize);
 *
 * This function allocate a SysBios mailbox to specified CAN \a id and mailbox number.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     Mailbox number. Valid value is 0-31, inclusive
 * \param mailboxSize       Mailbox size
 *
 * \return true: successful; false: otherwise
 */
Bool CAN_tx_config(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 mailboxSize);


/*!
 * \page page_can
 * \subsection subsect_can_mailbox_register CAN_mailbox_register
 * <PRE>Bool CAN_mailbox_register(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, Mailbox_Handle pMB);</PRE>
 *
 * \fn Bool CAN_mailbox_register(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, Mailbox_Handle pMB)
 *
 * This function registers a SysBIOS mailbox handle to specified \a mailboxNumber of a specified peripheral \a id.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     Mailbox number. Valid value is 0-31, inclusive
 * \param pMB               SysBIOS mailbox handle for event and data notification
 *
 * \return true: successful; false: otherwise */
Bool CAN_mailbox_register(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, Mailbox_Handle pMB);


/*!
 * \page page_can
 * \subsection subsect_can_send CAN_send
 * <PRE>Bool CAN_mailbox_send(CAN_PERIPH_ID_T id, UInt16 mailboxNumber, UInt16 * pMsg, UInt16 len);</PRE>
 *
 * \fn Bool CAN_send(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket)
 *
 * This function sends message to a specified mailbox of a specified CAN peripheral.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param pMsg              Pointer to CAN packet to send
 *
 * \return true: successful; false: otherwise
 */
Bool CAN_send(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket);


/*!
 * \page page_can
 * \subsection subsect_can_send_done CAN_send_done
 * <PRE>Bool CAN_send_done(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket);</PRE>
 *
 * \fn Bool CAN_send_done(CAN_PERIPH_ID_T id, CAN_MAILBOX_ENTRY_T * pCanPacket)
 *
 * This function returns true if all CAN tx in the mailbox has been sent.
 *
 * \param id                CAN peripheral instance ID (see ::CAN_PERIPH_ID_T)
 * \param mailboxNumber     CAN mailbox number used for transmission
 *
 * \return true: All messages are sent; false: otherwise
 */
Bool CAN_send_done(CAN_PERIPH_ID_T id, UInt16 mailboxNumber);


#endif /* CONFIG_USE_CAN || __DOXYGEN__ */
#endif /* COMM_CAN_H */
