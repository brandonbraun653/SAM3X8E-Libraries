/*
 * ioController.cpp
 *
 * Created: 11/23/2015 8:13:04 PM
 *  Author: Brandon
 */ 

#include "../libraries.h"


//Digital I/O
void pinMode(uint32_t pin, uint32_t value){
	switch (value){
		case OUTPUT:
		PIO_configurePin(
		pinCharacteristic[pin].port,
		pinCharacteristic[pin].pinMask,
		PIO_OUTPUT_1,
		pinCharacteristic[pin].pinConfiguration, NONE);
		
		break;
		
		case INPUT:
		PIO_configurePin(
		pinCharacteristic[pin].port,
		pinCharacteristic[pin].pinMask,
		PIO_INPUT,
		pinCharacteristic[pin].pinConfiguration, NONE);
		
		break;
		
		case INPUT_PULLUP:
		PIO_configurePin(
		pinCharacteristic[pin].port,
		pinCharacteristic[pin].pinMask,
		PIO_INPUT,
		PIO_PULLUP, NONE);
		break;
		
		default: break;
	}
}

uint32_t digitalRead(uint32_t pin, uint32_t dataReadType){
	switch (dataReadType){
		case OUTPUT_DATA:
		return PIO_readOutputDataStatus(pinCharacteristic[pin].port, pinCharacteristic[pin].pinMask);
		break;
		
		case PIN_DATA:
		return PIO_readPinDataStatus(pinCharacteristic[pin].port, pinCharacteristic[pin].pinMask);
		break;
		
		default: return 0xFFFFFFFF;
	}
}

void digitalWrite(uint32_t pin, uint32_t value){
	switch (value){
		case HIGH:
		PIO_setPin(pinCharacteristic[pin].port, pinCharacteristic[pin].pinMask);
		break;
		
		case LOW:
		PIO_clearPin(pinCharacteristic[pin].port, pinCharacteristic[pin].pinMask);
		break;
		
		default: return;
	}
}

uint32_t lookUp_Mask(uint32_t pin){
	return pinCharacteristic[pin].pinMask;
}

Pio* lookUp_Port(uint32_t pin){
	return pinCharacteristic[pin].port;
}

//Analog I/O
void analogReference(uint32_t value){
	
}

uint32_t analogRead(uint32_t pin){
	return 0xFFFFFFFF;
}

void analogWrite(uint32_t pin){
	
}
