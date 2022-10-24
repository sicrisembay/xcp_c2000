/*!
 * \file ringbuffer.h
 */
#ifndef SOURCE_UTILITY_RING_BUFFER_H_
#define SOURCE_UTILITY_RING_BUFFER_H_

#include <xdc/std.h>
#include <ti/sysbios/gates/GateHwi.h>

/*!
 * The type which is used to hold the size
 * and the indicies of the buffer.
 */
typedef UInt16 ring_buffer_size_t;

/*!
 * Structure which holds a ring buffer.
 * The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */
typedef struct {
  Char * pBuffer;                   /*!< Pointer to buffer memory */
  ring_buffer_size_t buffer_mask;   /*!< Mask used as a modulo operator */
  ring_buffer_size_t tail_index;    /*!< Index of tail. */
  ring_buffer_size_t head_index;    /*!< Index of head. */
  GateHwi_Handle hdl_gate;          /*!< SysBIOS HWI gate handle */
} ring_buffer_t;

/*!
 * Initializes the ring buffer pointed to by <em>pRingBuf</em>.
 * This function can also be used to empty/reset the buffer.
 *
 * \param pRingBuf  The ring buffer to initialize.
 * \param pBuf      Pointer to pre-allocated buffer
 * \size  size      Size of pre-allocated buffer
 *
 * \return None
 */
void ring_buffer_init(ring_buffer_t *pRingBuf, Char *pBuf, ring_buffer_size_t size);


/*!
 * Adds a byte to a ring buffer.
 *
 * \param pRingBuf  The ring buffer in which the data should be placed.
 * \param data      The data to place.
 *
 * \return None
 */
void ring_buffer_queue(ring_buffer_t *pRingBuf, Char data);


/*!
 * Adds an array of bytes to a ring buffer.
 *
 * \param pRingBuf  The ring buffer in which the data should be placed.
 * \param data      A pointer to the array of bytes to place in the queue.
 * \param size      The size of the array.
 *
 * \return None
 */
void ring_buffer_queue_arr(ring_buffer_t *pRingBuf, const Char *data, ring_buffer_size_t size);


/*!
 * Returns the oldest byte in a ring buffer.
 *
 * \param pRingBuf  The ring buffer from which the data should be returned.
 * \param data      A pointer to the location at which the data should be placed.
 *
 * \return 1 if data was returned; 0 otherwise.
 */
UInt8 ring_buffer_dequeue(ring_buffer_t *pRingBuf, Char *data);


/*!
 * Returns the <em>len</em> oldest bytes in a ring buffer.
 *
 * \param pRingBuf  The ring buffer from which the data should be returned.
 * \param data      A pointer to the array at which the data should be placed.
 * \param len       The maximum number of bytes to return.
 *
 * \return The number of bytes returned.
 */
ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *pRingBuf, Char *data, ring_buffer_size_t len);


/*!
 * Peeks a ring buffer, i.e. returns an element without removing it.
 *
 * \param pRingBuf  The ring buffer from which the data should be returned.
 * \param data      A pointer to the location at which the data should be placed.
 * \param index     The index to peek.
 *
 * \return 1 if data was returned; 0 otherwise.
 */
UInt8 ring_buffer_peek(ring_buffer_t *pRingBuf, Char *data, ring_buffer_size_t index);


/*!
 * Returns whether a ring buffer is empty.
 *
 * \param pRingBuf  The ring buffer for which it should be returned whether it is empty.
 *
 * \return 1 if empty; 0 otherwise.
 */
#pragma CODE_SECTION(ring_buffer_is_empty, "ramfuncs");  // Need to place in RAM as this is used inside ISR
inline UInt8 ring_buffer_is_empty(ring_buffer_t *pRingBuf) {
    return (pRingBuf->head_index == pRingBuf->tail_index);
}


/*!
 * Returns whether a ring buffer is full.
 *
 * \param pRingBuf  The ring buffer for which it should be returned whether it is full.
 *
 * \return 1 if full; 0 otherwise.
 */
#pragma CODE_SECTION(ring_buffer_is_full, "ramfuncs");  // Need to place in RAM as this is used inside ISR
inline UInt8 ring_buffer_is_full(ring_buffer_t *pRingBuf) {
    return ((pRingBuf->head_index - pRingBuf->tail_index) & (pRingBuf->buffer_mask)) == (pRingBuf->buffer_mask);
}


/*!
 * Returns the number of items in a ring buffer.
 *
 * \param buffer The buffer for which the number of items should be returned.
 *
 * \return The number of items in the ring buffer.
 */
inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer) {
    return ((buffer->head_index - buffer->tail_index) & (buffer->buffer_mask));
}

#endif /* SOURCE_UTILITY_RING_BUFFER_H_ */
