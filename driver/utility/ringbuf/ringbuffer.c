/*!
 * \file ringbuffer.c
 *
 * Implementation of ring buffer functions.
 */

#include "ringbuffer.h"
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>


void ring_buffer_init(ring_buffer_t *pRingBuf, Char *pBuf, ring_buffer_size_t size)
{
    Error_Block eb;
    GateHwi_Params gate_hwi_param;
    Error_init(&eb);
    GateHwi_Params_init(&gate_hwi_param);

    /* size MUST be a power of two */
    Assert_isTrue((pRingBuf != NULL) && (pBuf != NULL) &&
                  (size & (size - 1)) == 0, NULL);

    pRingBuf->pBuffer = pBuf;
    pRingBuf->buffer_mask = size - 1;
    pRingBuf->tail_index = 0;
    pRingBuf->head_index = 0;
    pRingBuf->hdl_gate = GateHwi_create(&gate_hwi_param, &eb);
    Assert_isTrue((Error_check(&eb) == FALSE) &&
                  (pRingBuf->hdl_gate != NULL), NULL);
}


#pragma CODE_SECTION(ring_buffer_queue, "ramfuncs");  // Need to place in RAM as this is used inside ISR
void ring_buffer_queue(ring_buffer_t *pRingBuf, Char data)
{
    IArg key;

    /* Is buffer full? */
    if(ring_buffer_is_full(pRingBuf)) {
        /* Is going to overwrite the oldest byte */
        /* Increase tail index */
        key = GateHwi_enter(pRingBuf->hdl_gate);
        pRingBuf->tail_index = ((pRingBuf->tail_index + 1) & (pRingBuf->buffer_mask));
        GateHwi_leave(pRingBuf->hdl_gate, key);
    }

    /* Place data in buffer */
    pRingBuf->pBuffer[pRingBuf->head_index] = data;
    key = GateHwi_enter(pRingBuf->hdl_gate);
    pRingBuf->head_index = ((pRingBuf->head_index + 1) & (pRingBuf->buffer_mask));
    GateHwi_leave(pRingBuf->hdl_gate, key);
}


void ring_buffer_queue_arr(ring_buffer_t *pRingBuf, const Char *data, ring_buffer_size_t size)
{
    /* Add bytes; one by one */
    ring_buffer_size_t i;
    for(i = 0; i < size; i++) {
        ring_buffer_queue(pRingBuf, data[i]);
    }
}


#pragma CODE_SECTION(ring_buffer_dequeue, "ramfuncs");  // Need to place in RAM as this is used inside ISR
UInt8 ring_buffer_dequeue(ring_buffer_t *pRingBuf, Char *data)
{
    IArg key;

    if(ring_buffer_is_empty(pRingBuf)) {
        /* No items */
        return 0;
    }
  
    *data = pRingBuf->pBuffer[pRingBuf->tail_index];
    key = GateHwi_enter(pRingBuf->hdl_gate);
    pRingBuf->tail_index = ((pRingBuf->tail_index + 1) & (pRingBuf->buffer_mask));
    GateHwi_leave(pRingBuf->hdl_gate, key);
    return 1;
}


ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *pRingBuf, Char *data, ring_buffer_size_t len)
{
    if(ring_buffer_is_empty(pRingBuf)) {
        /* No items */
        return 0;
    }

    Char * data_ptr = data;
    ring_buffer_size_t cnt = 0;
    while((cnt < len) && ring_buffer_dequeue(pRingBuf, data_ptr)) {
        cnt++;
        data_ptr++;
    }
    return cnt;
}


UInt8 ring_buffer_peek(ring_buffer_t *pRingBuf, Char *data, ring_buffer_size_t index)
{
    IArg key;

    if(index >= ring_buffer_num_items(pRingBuf)) {
        /* No items at index */
        return 0;
    }
  
    /* Add index to pointer */
    key = GateHwi_enter(pRingBuf->hdl_gate);
    ring_buffer_size_t data_index = ((pRingBuf->tail_index + index) & (pRingBuf->buffer_mask));
    GateHwi_leave(pRingBuf->hdl_gate, key);
    *data = pRingBuf->pBuffer[data_index];
    return 1;
}


extern inline UInt8 ring_buffer_is_empty(ring_buffer_t *buffer);
extern inline UInt8 ring_buffer_is_full(ring_buffer_t *buffer);
extern inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer);

