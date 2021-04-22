/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include "ring_buffer.h"


bool RingBuffer_Init(RingBuffer *ringBuffer, char *dataBuffer, size_t dataBufferSize) 
{
	assert(ringBuffer);
	assert(dataBuffer);
	assert(dataBufferSize > 0);
	
	if ((ringBuffer) && (dataBuffer) && (dataBufferSize > 0)) {
		*ringBuffer = (RingBuffer) {
			.buf = dataBuffer,
			.head = dataBuffer,
			.tail = dataBuffer,
			.cap = dataBufferSize
		};
		RingBuffer_Clear(ringBuffer);
		return true;
	}
	return false;
}

bool RingBuffer_Clear(RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		for (volatile uint16_t i = 0; i < ringBuffer->cap; i++)
			ringBuffer->buf[i] = 0;
		ringBuffer->count = 0;
		return true;
	}
	return false;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer)
{
  assert(ringBuffer);	
	if (ringBuffer->count == 0)
		return true;
	return false;
}

size_t RingBuffer_GetLen(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		return ringBuffer->count;
	}
	return 0;
	
}

size_t RingBuffer_GetCapacity(const RingBuffer *ringBuffer)
{
	assert(ringBuffer);
	
	if (ringBuffer) {
		return ringBuffer->cap;
	}
	return 0;	
}


bool RingBuffer_PutChar(RingBuffer *ringBuffer, char c)
{
	assert(ringBuffer);
	
	if (ringBuffer && ringBuffer->count < ringBuffer->cap) {
		ringBuffer->count++;
		*ringBuffer->head++ = c;
		if (ringBuffer->head == ringBuffer->buf + ringBuffer->cap)
			ringBuffer->head = ringBuffer->buf;
		
		return true;
	}
	return false;
}

bool RingBuffer_GetChar(RingBuffer *ringBuffer, char *c)
{
	assert(ringBuffer);
	assert(c);
	
	if ((ringBuffer) && (c) && ringBuffer->tail >= ringBuffer->buf) {
		ringBuffer->count--;
		*c = *ringBuffer->tail++;
		if (ringBuffer->tail == ringBuffer->buf + ringBuffer->cap)
			ringBuffer->tail = ringBuffer->buf;
			
		return true;
	}
	return false;
}
