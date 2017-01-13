/*
 * ringbuffer.h
 *
 * Created: 12/18/2015 5:26:01 PM
 *  Author: Brandon
 */ 


#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include "../libraries.h"

class RingBufferClass{
public:
	RingBufferClass(void);
	RingBufferClass(uint32_t *buffer, uint32_t bufferLength);
	uint32_t	write(uint32_t data);
	uint32_t	write(uint32_t *dataBuffer, uint32_t numToWrite);
	uint32_t	read();
	uint32_t	peek();
	uint32_t	availableToRead();
	uint32_t	availableToWrite();
	void		flush();
	
private:
	static const uint32_t DEFAULT_BUFFER_SIZE = 16;
	volatile uint32_t *ringBuffer;
	volatile uint32_t bufferSize;
	volatile uint32_t readLocation;
	volatile uint32_t writeLocation;
	volatile uint32_t bufferedData;
};

#endif /* RINGBUFFER_H_ */