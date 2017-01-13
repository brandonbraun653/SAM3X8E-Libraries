/*
 * timer.cpp
 *
 * Created: 12/8/2016 9:51:51 PM
 *  Author: codex
 */ 

#include "../libraries.h"


/************************************************************************/
/* SYSTEM TICK CLASS FUNCTIONS                                          */
/************************************************************************/

//Constructor to configure basic instance information
SystemTickClass::SystemTickClass(uint32_t channel, uint32_t frequency){
	switch (channel)
	{
	/* Timer Counter 0 */
	case 0:
		channelNum = 0;
		periphID = ID_TC0;
		IRQnID = TC0_IRQn;
		instanceID = TC0;
		break;
		
	case 1:
		channelNum = 1;
		periphID = ID_TC1;
		IRQnID = TC1_IRQn;
		instanceID = TC0;
		break;
	
	case 2:
		channelNum = 2;
		periphID = ID_TC2;
		IRQnID = TC2_IRQn;
		instanceID = TC0;
		break;
		
	/* Timer Counter 1 */
	case 3:
		channelNum = 0;
		periphID = ID_TC3;
		IRQnID = TC3_IRQn;
		instanceID = TC1;
		break;
	
	case 4:
		channelNum = 1;
		periphID = ID_TC4;
		IRQnID = TC4_IRQn;
		instanceID = TC1;
		break;
	
	case 5:
		channelNum = 2;
		periphID = ID_TC5;
		IRQnID = TC5_IRQn;
		instanceID = TC1;
		break;
	
	/* Timer Counter 2 */
	case 6:
		channelNum = 0;
		periphID = ID_TC6;
		IRQnID = TC6_IRQn;
		instanceID = TC2;
		break;
	
	case 7:
		channelNum = 1;
		periphID = ID_TC7;
		IRQnID = TC7_IRQn;
		instanceID = TC2;
		break;
	
	case 8:
		channelNum = 2;
		periphID = ID_TC8;
		IRQnID = TC8_IRQn;
		instanceID = TC2;
		break;	
	default:
		break;
	}
	
	channelFreq = frequency;
	tickTock = 0;
	
	//Initialize and start the timer
	this->initializeTick();
	this->start();
}

//Interrupt call function
void SystemTickClass::IRQHandler(){
	//Read the status register so the next interrupt can occur
	TC_GetStatus(instanceID, channelNum);
	
	//Increase the internal counter
	++tickTock;
	
	//If attached interrupt function exists, call it
	if(onInterruptCallback)
		onInterruptCallback();
};

//Initialize the timer with given period and units
void SystemTickClass::initializeTick(){
	//Enable peripheral clock
	pmc_enable_periph_clk(periphID);
	
	//Enable interrupts
	NVIC_DisableIRQ(IRQnID);
	NVIC_ClearPendingIRQ(IRQnID);
	NVIC_SetPriority(IRQnID, (uint32_t)PRIOR_TIMER);
	NVIC_EnableIRQ(IRQnID);
	
	//Find the best clock divider for given frequency
	TC_FindMckDivisor(channelFreq, MASTER_CLOCK, divisor, tcClocks, MASTER_CLOCK);
	
	uint32_t clockVal, divider;
	switch ((uint32_t)*divisor)
	{
	case 2:
		clockVal = TC_CMR_TCCLKS_TIMER_CLOCK1;
		divider = 2;
		break;
	
	case 8:
		clockVal = TC_CMR_TCCLKS_TIMER_CLOCK2;
		divider = 8;
		break;
	
	case 32:
		clockVal = TC_CMR_TCCLKS_TIMER_CLOCK3;
		divider = 32;
		break;
	
	case 128:
		clockVal = TC_CMR_TCCLKS_TIMER_CLOCK4;
		divider = 128;
		break;
	default: //Highest resolution
		clockVal = TC_CMR_TCCLKS_TIMER_CLOCK1;
		divider = 2;
		break;
	}
	
	
	//Set counter mode and behavior
	uint32_t chMode = (clockVal | TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC);
	TC_Configure(instanceID, channelNum, chMode);
	
	//Set counter value
	uint32_t countMax = MASTER_CLOCK / divider / channelFreq;
	TC_SetRC(instanceID, channelNum, countMax);

	//Set interrupts
	instanceID->TC_CHANNEL[channelNum].TC_IER = TC_IER_CPCS;
	instanceID->TC_CHANNEL[channelNum].TC_IDR = ~TC_IER_CPCS;	
}

//Start timer from last location
void SystemTickClass::start(){
	TC_Start(instanceID, channelNum);
}

//Stop the timer. Resets counter register but not tickTock count
void SystemTickClass::stop(){
	TC_Stop(instanceID, channelNum);
}

//Returns current value of tickTock
uint32_t SystemTickClass::tickCount(){
	//Disable channel interrupt
	instanceID->TC_CHANNEL[channelNum].TC_IDR |= TC_IER_CPCS;
	
	//Copy the current tick value
	uint32_t tempTick = tickTock;
	
	//Re-enable channel interrupt
	instanceID->TC_CHANNEL[channelNum].TC_IER |= TC_IER_CPCS;
	
	return tempTick;
}

//Assigns user defined interrupt event function
void SystemTickClass::onInterrupt(void(*function)(void)){
	onInterruptCallback = function;
}

//////////////////////////////////////////////////////////////////////////

/************************************************************************/
/* EVENT CAPTURE CLASS FUNCTIONS                                        */
/************************************************************************/
EventCaptureClass::EventCaptureClass(uint32_t channel, uint32_t maxFreq, CaptureType captureMode, bool filter){
	switch (channel)
	{
	case 0: //Will break any MILLIS() calls if used.
		hardwareChannel = 0;
		periphID = ID_TC0;
		IRQnID = TC0_IRQn;
		instanceID = TC0;
		break;
	
	case 6:
		hardwareChannel = 0;
		periphID = ID_TC6;
		IRQnID = TC6_IRQn;
		instanceID = TC2;
		break;
	
	case 7:
		hardwareChannel = 1;
		periphID = ID_TC7;
		IRQnID = TC7_IRQn;
		instanceID = TC2;
		break;
	
	case 8:
		hardwareChannel = 2;
		periphID = ID_TC8;
		IRQnID = TC8_IRQn;
		instanceID = TC2;
		break;
	
	//Can't use channels 1-5. Not connected or used by ADC periph
	default: return;
	}
	
	
	fMax = maxFreq;
	filterSignal = filter;
	timerChannel = channel;
	edgeDetectionMode = captureMode;
	
	//Initialize the timePerTick value to 0 in fix16_t format
	sig.timePerTick = fix16_from_int(0u);
}

bool EventCaptureClass::begin(){
	if(initialize())
		return true;
	else 
		return false;
}

void EventCaptureClass::pause(){
	disableInterrupts();
}

void EventCaptureClass::restart(){
	enableInterrupts();
}

void EventCaptureClass::end(){
	disableInterrupts();
	hw_initialized = false;
}

void EventCaptureClass::sampleSignal(){
	//Pause the input capture unit so that we don't collide with interrupt
	disableInterrupts();
	
	/** Check for overflow since last read (highly unlikely) **/
	if((status & TC_SR_COVFS) == TC_SR_COVFS)
		return; 
	
/** Main calculations here, based on 1.5 periods of signal info.
	
	The signals increase in timestamp according to this order:
		
		  (rise)      (fall)     (rise)  (fall)
		regA_PAST -> regB_PAST -> regA -> regB 
		 (oldest)						(newest)
**/
	
	/** UPDATE NUMBER OF EDGES SEEN **/	
	InputSignal.numRisingEdges  = rEdge;
	InputSignal.numFallingEdges = fEdge;
	
	
	/** CALCULATE MAIN SIGNAL INFORMATION **/
	
	//Rising edge triggers period start: only RA updates
	if(edgeDetectionMode == RISING){
		sig.deltaRising = fix16_from_int(regA - regA_PAST);
		
		//Divide clock speed by number of counts = freq;
		
		fix16_t freq = fix16_div(fix16_from_int(10500000), sig.deltaRising);
		
		Serial1.write(fix16_to_int(freq));
	}
	
	//Falling edge triggers period start: only RB updates
	if(edgeDetectionMode == FALLING){

	}
	
	//Both edges trigger interrupt: RA and RB data is updated
	if(edgeDetectionMode == EDGE){
		////How do you deal with signal calculations if only rising or
		//falling edge selected? Duty cycle needs both edges. Set zero in calcs
	}
	
	//Re-enable the input capture
	enableInterrupts();
}

void EventCaptureClass::IRQHandler(){
	//Read status register
	status = instanceID->TC_CHANNEL[hardwareChannel].TC_SR & TC_SR_MTIOA;
	
	//Rising edge detected
	if(status){
		//Store the last read value
		regA_PAST = regA;
		
		//Record the new value
		regA = instanceID->TC_CHANNEL[hardwareChannel].TC_RA;
		
		//Increment the number of rising edges detected
		++rEdge;
		
		//Callback functions: Try not to use these in high freq measurements
// 		if(onRisingCallback)
// 		onRisingCallback();
// 		
// 		if(onEdgeCallback)
// 		onEdgeCallback();
		
		return;
	}
	
	//Falling edge detected
	if(!status){
		//Store the last read value
		regB_PAST = regB;
		
		//Record the new value
		regB = instanceID->TC_CHANNEL[hardwareChannel].TC_RB;
		
		//Increment the number of falling edges detected
		++fEdge;
				
		//Callback functions: Try not to use these in high freq measurements
// 		if(onFallingCallback)
// 		onFallingCallback();
// 				
// 		if(onEdgeCallback)
// 		onEdgeCallback();
		
		return;
	}
	
	
	
	




	
}

void EventCaptureClass::enableInterrupts(){
	instanceID->TC_CHANNEL[hardwareChannel].TC_IER |= interruptMask;
}

void EventCaptureClass::disableInterrupts(){
	instanceID->TC_CHANNEL[hardwareChannel].TC_IDR |= interruptMask;
	NVIC_ClearPendingIRQ(IRQnID);
}

bool EventCaptureClass::initialize(){
	//Peripheral Clock
	pmc_enable_periph_clk(periphID);
	
	//Interrupts
	NVIC_DisableIRQ(IRQnID);
	NVIC_ClearPendingIRQ(IRQnID);
	NVIC_SetPriority(IRQnID, (uint32_t)PRIOR_TIMER);
	NVIC_EnableIRQ(IRQnID);
	
	//PIO
	PIO_configurePin(
	pinCharacteristic[TIOA[timerChannel]].port,
	pinCharacteristic[TIOA[timerChannel]].pinMask,
	pinCharacteristic[TIOA[timerChannel]].peripheralType,
	pinCharacteristic[TIOA[timerChannel]].pinAttribute, INPUT);
	
	/** Clear all previous config data **/
	instanceID->TC_CHANNEL[hardwareChannel].TC_CCR = 0u;
	instanceID->TC_CHANNEL[hardwareChannel].TC_CMR = 0u;
	instanceID->TC_CHANNEL[hardwareChannel].TC_IDR = 0xFFFFFFFF;
	
	if(calculateClock() == false)
		return false; //Uh oh, input signal freq way too high
	
	/** Setup new config **/
	instanceID->TC_CHANNEL[hardwareChannel].TC_CMR = clockSelect | TC_CMR_LDRA_RISING | TC_CMR_LDRB_FALLING;
	//^^^Capture mode implicitly defined with WAVE = 0
	
	if(calculateTimePerTick() == false)
		return false;
		
	/** Choose which interrupts to fire **/
	if(edgeDetectionMode == RISING){
		instanceID->TC_CHANNEL[hardwareChannel].TC_IER = TC_IER_LDRAS;
		interruptMask = TC_IER_LDRAS;
		
	} else if(edgeDetectionMode == FALLING){
		instanceID->TC_CHANNEL[hardwareChannel].TC_IER = TC_IER_LDRBS;
		interruptMask = TC_IER_LDRBS;
		
	} else if(edgeDetectionMode == EDGE){
		instanceID->TC_CHANNEL[hardwareChannel].TC_IER = TC_IER_LDRAS | TC_IER_LDRBS;
		interruptMask = TC_IER_LDRAS | TC_IER_LDRBS;
		
	} else { //DISABLED
		instanceID->TC_CHANNEL[hardwareChannel].TC_IDR = TC_IDR_LDRAS | TC_IDR_LDRBS;
		interruptMask = 0u;
	}
	
	
	/** Reset counter and enable the clock **/
	instanceID->TC_CHANNEL[hardwareChannel].TC_CCR = TC_CCR_SWTRG | TC_CCR_CLKEN;
	
	return true; //Success
}

bool EventCaptureClass::calculateClock(){
/** 
There are four main options for a clock select:
	TIMER_CLOCK_1 = MCK/2   = 84,000,000/2   =  48.000 MHz =   20.83333 nS/tick
	TIMER_CLOCK_2 = MCK/8   = 84,000,000/8   =  10.500 MHz =   95.23809 nS/tick
	TIMER_CLOCK_3 = MCK/32  = 84,000,000/32  =   2.625 MHz =  380.95238 nS/tick
	TIMER_CLOCK_4 = MCK/128 = 84,000,000/128 = 656.250 kHz = 1523.80952 nS/tick
	
For the most basic interrupt call, the CPU will take 2uS to detect the edge transition, execute the interrupt, and return
back to normal program operation. This limits the max frequency that can be detected by the input capture unit in current
configuration.
	
	fMaxIdeal = 1/2uS = 500 kHz (50% duty cycle, single edge detection)

This is assuming the CPU is doing literally nothing else but detecting this signal continuously. Of course the input capture
unit can be enabled/disabled to effectively sample the channel periodically and free up the CPU. Regardless, this frequency 
cannot be exceed easily. This value is HIGHLY optimistic.

The clock select effectively sets a resolution on signal measurement. To get a fair amount of calculation accuracy, 100 clocks 
should occur between the loading of RA and RB. This limits the frequencies that can be measured on each prescaler. The table 
below details how the code will setup the TCCLKS register based on expected input frequencies. Any input frequency that falls
below the range specified will "suffer" increased resolution in calculations. This is hardly a problem.

	TIMER_CLOCK_1 (Ultra High Frequencies): 200 kHz -  400 kHz
	TIMER_CLOCK_2 (High Frequencies):        50 kHz -  200 kHz
	TIMER_CLOCK_3 (Mid Frequencies):		 10 kHz -   50 kHz
	TIMER_CLOCK_4 (Low Frequencies):		  0  Hz -   10 kHz

Please keep in mind that if a frequency that is higher than the max specified occurs, it will still measure the signal BUT at 
the cost of decreased resolution. If using this module in a sampling mode for DSP calculations, it would be useful to put a 
software filter on the signal to reject low resolution high frequency content.
**/
	
	if(fMax < 10000u)
		clockSelect = TC_CMR_TCCLKS_TIMER_CLOCK4;

	else if(fMax >= 10000u && fMax < 50000u)
		clockSelect = TC_CMR_TCCLKS_TIMER_CLOCK3;

	else if(fMax >= 50000u && fMax < 200000u)
		clockSelect = TC_CMR_TCCLKS_TIMER_CLOCK2;
		
	else if(fMax >= 200000u && fMax < 400000u)
		clockSelect = TC_CMR_TCCLKS_TIMER_CLOCK1;
		
	else
		return false; //Input fMax waaaay above what we can handle.
	
	//Good. We can measure the input signal.
	return true;
}

bool EventCaptureClass::calculateTimePerTick(){
	//Must be called after calculateClock(). Values in uS
	switch (clockSelect)
	{
	case 0u:
		sig.timePerTick = fix16_from_float(0.02083333);
		return true;
		break;
	
	case 1u:
		sig.timePerTick = fix16_from_float(95.23809);
		return true;
		break;
	
	case 2u:
		sig.timePerTick = fix16_from_float(0.38095238);
		return true;
		break;
	
	case 3u:
		sig.timePerTick = fix16_from_float(1.52380952);
		return true;
		break;
	
	default: return false;
	}
}

void EventCaptureClass::onRising(void(*function)(void)){
	onRisingCallback = function;
}

void EventCaptureClass::onFalling(void(*function)(void)){
	onFallingCallback = function;
}

void EventCaptureClass::onEdge(void(*function)(void)){
	onEdgeCallback = function;
}

/************************************************************************/
/* EXPORTED FUNCTIONS                                                   */
/************************************************************************/

/* Abstract delay function that can accommodate multiple units */
extern void delay(uint32_t amount, Time units){
	switch (units)
	{
	case MilliSeconds:
		delayMilliseconds(amount);
		break;
		
	case MicroSeconds:
		
		break;
	
	case Seconds:
		
		break;
	
	case NanoSeconds:
		
		break;
		
	default: //Do nothing
		break;
	}
}

extern void delaySeconds(uint32_t S){
	//Nothing yet
}

extern void delayMilliseconds(uint32_t mS){
	uint32_t currentTime = millis();
	while(millis()-currentTime < mS); //Wait for time to expire
}

extern void delayMicroseconds(uint32_t uS){
	//Nothing yet
}

/* Returns the number of seconds elapsed since program start.
   1 Second Resolution.*/
extern uint32_t seconds(){
	return (uint32_t)round(millis()/1000);
}

/* Returns number of milliseconds elapsed since program start.
   1 Millisecond Resolution*/
extern uint32_t millis(){
	return milliSysTick.tickCount();
}

/* Returns number of microseconds elapsed since program start.
   Resolution TBD */
extern uint32_t micros(){
	//TO DO
	return 0;
}

