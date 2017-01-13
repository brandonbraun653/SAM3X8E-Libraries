/*
 * rng.cpp
 *
 * Created: 12/31/2016 11:51:57 PM
 *  Author: codex
 */ 

#include "../include/rng.h"

void RNG_start(){
	TRNG->TRNG_CR = RNG_KEY | TRNG_CR_ENABLE;
}

void RNG_stop(){
	TRNG->TRNG_CR = RNG_KEY | ~TRNG_CR_ENABLE;
}

void RNG_enableIT(){
	TRNG->TRNG_IER = 0x1u;
}

void RNG_disableIT(){
	TRNG->TRNG_IDR = 0x1u;
}

uint32_t RNG_generate(){
	return TRNG->TRNG_ODATA;
}