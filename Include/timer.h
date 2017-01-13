/*
 * timer.h
 *
 * Created: 12/8/2016 9:52:04 PM
 *  Author: codex
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include "../libraries.h"
#include "definitions.h"

//Needed to do math on decimal numbers. ARM-M3 has no FPU.
#include "../Tertiary/FixPointMath/fix16.h"
#include "../Tertiary/FixPointMath/fix16.hpp"

class SystemTickClass
{
public:
	SystemTickClass(uint32_t channel, uint32_t frequency);
	
	void initializeTick();
	void start();
	void stop();
	uint32_t tickCount();
	
	//Interrupt Request Handler
	void IRQHandler(void);
	
	//Allows user to attach function to interrupt
	void onInterrupt(void(*)(void));
	
protected:
private:
	
	//Keeps track of the current tick value
	volatile uint32_t tickTock;
	
	IRQn_Type IRQnID;
	Tc *instanceID;
	uint32_t periphID;
	uint32_t channelNum;
	uint32_t channelFreq;
	uint32_t *divisor;	//Divisor value chosen for mode register
	uint32_t *tcClocks;	//TCCLKS field value for mode register
	
	//Callback user functions
	void (*onInterruptCallback)(void);
	
/*	Note: Channel Mapping Table
TC   Chan   NVIC    IRQ function   PMC id
TC0   0   TC0_IRQn   TC0_Handler   ID_TC0
TC0   1   TC1_IRQn   TC1_Handler   ID_TC1
TC0   2   TC2_IRQn   TC2_Handler   ID_TC2
TC1   0   TC3_IRQn   TC3_Handler   ID_TC3
TC1   1   TC4_IRQn   TC4_Handler   ID_TC4
TC1   2   TC5_IRQn   TC5_Handler   ID_TC5
TC2   0   TC6_IRQn   TC6_Handler   ID_TC6
TC2   1   TC7_IRQn   TC7_Handler   ID_TC7
TC2   2   TC8_IRQn   TC8_Handler   ID_TC8
*/
};

class EventCaptureClass
{
public:
	bool begin();
	void pause();
	void restart();
	void end();
	void sampleSignal();
	
	/** User defined functions attached to input level changes **/
	void onRising(void(*)(void));
	void onFalling(void(*)(void));
	void onEdge(void(*)(void));
	
	/** Interrupt Handler **/
	void IRQHandler();
	
	/** Constructor **/
	EventCaptureClass(uint32_t channel, uint32_t maxFreq, CaptureType captureMode = DISABLED, bool filter = false);
	
	//Pulse Information
	struct SignalInfo{
		uint32_t frequency;
		uint32_t period;
		uint32_t dutyCycle;
		uint32_t timeHigh;
		uint32_t timeLow;
		uint32_t numRisingEdges;
		uint32_t numFallingEdges;
	};
	SignalInfo InputSignal;

private:
	//Hardware Information
	Tc *instanceID;
	IRQn_Type IRQnID;
	uint32_t periphID;
	uint32_t hardwareChannel;
	uint32_t TIOA[9] = {2, 92, 92, 92, 92, 92, 5 , 3, 11}; //92 is empty in PIO.cpp
	uint32_t TIOB[9] = {13, 92, 92, 92, 92, 92, 4, 10, 12};
	
	//General Variables
	bool filterSignal = false;				//Flag to enable/disable signal parameter averages
	bool hw_initialized = false;			//Flag to tell if hardware is ready
	uint32_t fMax;							//Input max frequency expected to measure
	uint32_t timerChannel;					//Actual timer channel (0-8) used
	uint32_t interruptMask;					//Interrupt associated with edge detection type	
	uint32_t clockSelect;					//Clock prescaler chosen based on max freq
	CaptureType edgeDetectionMode;			//Stores inputted edge detection type
	
	//Interrupt Variables
	volatile uint32_t regA, regB, status;	//Register values
	volatile uint32_t rEdge, fEdge;			//Edge transition counter
	volatile uint32_t regA_PAST, regB_PAST; //Last period's values
	
	//Fixed point math data
	struct SignalInfoFix16{
		fix16_t deltaRising;
		fix16_t deltaFalling;
		fix16_t dutyCycle;
		fix16_t frequency;
		fix16_t period;
		fix16_t timeHigh;
		fix16_t timeLow;
		fix16_t timePerTick;
	};
	SignalInfoFix16 sig;
	
	bool initialize();
	bool calculateClock();
	bool calculateTimePerTick();
	void enableInterrupts();
	void disableInterrupts();
	
	/** Attached user functions **/
	void (*onRisingCallback)(void);
	void (*onFallingCallback)(void);
	void (*onEdgeCallback)(void);
	
	
/**PIN MAPPING FOR CAPTURE MODE: UDOO QUAD REV. D
	Channel				captureModeA				captureModeB
				 (signal | I/O Line | dPin)	 (signal | I/O Line | dPin)
TC0	   0		   TIOA0     PB25	   2       TIOB0     PB27	   13
TC0	   1		   TIOA1     PA2	   NC      TIOB1     PA3       NC
TC0    2		   TIOA2     PA5	   NC      TIOB2     PA6       NC

TC1	   3		   TIOA3     PB0       NC      TIOB3     PB1       NC
TC1    4		   TIOA4     PB2       NC      TIOB4     PB3       NC
TC1    5		   TIOA5	 PB4       NC      TIOB5     PB5       NC

TC2    6		   TIOA6     PC25      5       TIOB6     PC26      4
TC2    7		   TIOA7	 PC28      3       TIOB7     PC29      10
TC2    8		   TIOA8     PD7       11      TIOB8     PD8	   12
**/
};
/************************************************************************/
/* EXPORTED FUNCTIONS                                                   */
/************************************************************************/
extern void delay(uint32_t amount, Time units);
extern void delaySeconds(uint32_t S);
extern void delayMilliseconds(uint32_t mS);
extern void delayMicroseconds(uint32_t uS);
extern uint32_t seconds();
extern uint32_t millis();
extern uint32_t micros();


#endif /* TIMER_H_ */