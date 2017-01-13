/*
 * ioController.h
 *
 * Created: 11/23/2015 8:18:21 PM
 *  Author: Brandon
 */ 


#ifndef IOCONTROLLER_H_
#define IOCONTROLLER_H_

#include "../libraries.h"

//Digital I/O
void pinMode(uint32_t pin, uint32_t value);
void digitalWrite(uint32_t pin, uint32_t value);
uint32_t digitalRead(uint32_t pin, uint32_t dataReadType);
uint32_t lookUp_Mask(uint32_t pin);
Pio* lookUp_Port(uint32_t pin);

//Analog I/O
void analogReference(uint32_t value);
uint32_t analogRead(uint32_t pin);
void analogWrite(uint32_t pin);




#endif /* IOCONTROLLER_H_ */