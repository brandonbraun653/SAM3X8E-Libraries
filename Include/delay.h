/*
 * delay.h
 *
 * Created: 12/19/2015 9:15:19 AM
 *  Author: Brandon
 */ 


#ifndef DELAY_H_
#define DELAY_H_

#include "../libraries.h"

extern uint32_t millis(void);

extern uint32_t micros(void);

extern void delay(uint32_t mS);

extern void delayMicroseconds(uint32_t uS);

#endif /* DELAY_H_ */