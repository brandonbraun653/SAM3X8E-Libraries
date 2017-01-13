/*
 * ringbuffer.cpp
 *
 * Created: 12/18/2015 5:29:18 PM
 *  Author: Brandon
 */ 

#include "../libraries.h"

//Constructor -- Unused except for friend classes
RingBufferClass::RingBufferClass(void){}

//Constructor -- Used for object instantiation
RingBufferClass::RingBufferClass(uint32_t *buffer, uint32_t bufferLength){
	writeLocation = 0;
	readLocation = 0;
	bufferedData = 0;
	
	bufferSize = bufferLength;
	ringBuffer = buffer;

	memset((void *)ringBuffer, 0, bufferSize);
}

//Destroys all data and sets buffer to zero
void RingBufferClass::flush(){
	readLocation = 0;
	writeLocation = 0;
	bufferedData = 0;
	memset((void *)ringBuffer, 0, bufferSize);
}

//Stores a single word to the ringBuffer
uint32_t RingBufferClass::write(uint32_t data){

	if(this->availableToWrite()){
		ringBuffer[writeLocation] = data;
		
		//Make sure the buffer fill count can't overrun bounds
		if((0<=bufferedData) && (bufferedData<bufferSize)){
			++bufferedData;
			writeLocation = (writeLocation + 1) % bufferSize;
		}
		
		return 1u; //Success	
	} else
		return 0u; //Buffer full
}

uint32_t RingBufferClass::write(uint32_t *dataBuffer, uint32_t numToWrite){
	//Grab how much data was given to write
	uint32_t inputLength = numToWrite;
	uint32_t error = 1u;
	
	//Is it larger than what the buffer can hold? 
	if(inputLength > bufferSize){
		inputLength = bufferSize;
		error = 0x10; //Input buffer oversize
	}
		
	//Write the data. Don't bother checking if there is enough room
	//because write() does that for you.
	for(uint32_t x=0; x<inputLength; x++){
		if(write(dataBuffer[x]) == 0u)
			error = 0x20; //Storage buffer ran out of space
	}
	
	return error;
}

//Reads a single word from the ringBuffer
uint32_t RingBufferClass::read(){

	//If any data can be read out..
	if(this->availableToRead()){
		uint32_t tempIndex = readLocation; //Save state
		

		//Make sure the buffer read count can't overrun bounds
		if((0<bufferedData) && (bufferedData<=bufferSize)){
			--bufferedData;
			readLocation = (readLocation + 1) % bufferSize;
		}
		return ringBuffer[tempIndex];
	} else
		return 0u; //Buffer empty
}

//Takes a peek at the value in the current readLocation without incrementing
uint32_t RingBufferClass::peek(){
	return ringBuffer[readLocation];
}

//Returns how many words can be read out of the buffer
uint32_t RingBufferClass::availableToRead(){
    return bufferedData;
}

//Returns how many words can be stored into the buffer
uint32_t RingBufferClass::availableToWrite(){
	return bufferSize - bufferedData;
}
