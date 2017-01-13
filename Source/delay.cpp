/*
 * delay.cpp
 *
 * Created: 12/19/2015 9:15:30 AM
 *  Author: Brandon
 */ 

#include "../libraries.h"

//Returns the number of milliseconds elapsed since program initialization
extern uint32_t millis(void){
	 return GetTickCount() ;
}

//Returns the number of microseconds elapsed since program initialization
extern uint32_t micros(void){
	uint32_t ticks, ticks2;
    uint32_t pend, pend2;
    uint32_t count, count2;

    ticks2  = SysTick->VAL;
    pend2   = !!((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)||((SCB->SHCSR & SCB_SHCSR_SYSTICKACT_Msk)))  ;
    count2  = GetTickCount();

    do {
        ticks=ticks2;
        pend=pend2;
        count=count2;
        ticks2  = SysTick->VAL;
        pend2   = !!((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)||((SCB->SHCSR & SCB_SHCSR_SYSTICKACT_Msk)))  ;
        count2  = GetTickCount();
    } while ((pend != pend2) || (count != count2) || (ticks < ticks2));

    return ((count+pend) * 1000) + (((SysTick->LOAD  - ticks)*(1048576/(VARIANT_MCK/1000000)))>>20) ; 
    // this is an optimization to turn a runtime division into two compile-time divisions and 
    // a runtime multiplication and shift, saving a few cycles
}

//Delays for mS number of milliseconds
extern void delay(uint32_t mS){
	
	//Currently not working. Not sure why.
	if (mS == 0)
	return;
	uint32_t start = GetTickCount();
	while (GetTickCount() - start < mS);
}

//Delays for uS number of microseconds
extern void delayMicroseconds(uint32_t uS){
	/*
     * Based on Paul Stoffregen's implementation
     * for Teensy 3.0 (http://www.pjrc.com/)
     */
    if (uS == 0) return;
    uint32_t n = uS * (VARIANT_MCK / 3000000);
    asm volatile(
        "L_%=_delayMicroseconds:"       "\n\t"
        "subs   %0, #1"                 "\n\t"
        "bne    L_%=_delayMicroseconds" "\n"
        : "+r" (n) :
    );
}
