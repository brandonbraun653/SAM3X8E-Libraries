/*
 * rng.h
 *
 * Created: 12/31/2016 11:52:12 PM
 *  Author: codex
 */ 


#ifndef RNG_H_
#define RNG_H_

#include "../libraries.h"

#define RNG_KEY 0x524E47

extern void RNG_start();
extern void RNG_stop();
extern void RNG_enableIT();
extern void RNG_disableIT();
extern uint32_t RNG_generate();


#endif /* RNG_H_ */