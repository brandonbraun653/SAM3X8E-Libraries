/*
 * libraries.cpp
 *
 * Created: 12/28/2015 2:58:45 PM
 *  Author: Brandon
 */ 

#include "libraries.h"

/************************************************************************/
/* Global Class Definitions                                             */
/************************************************************************/

/*****************
Task Scheduler
******************/
Scheduler taskList = Scheduler();

/*****************
UART Objects
******************/
UARTClass Serial;

//Interrupt Handler
void UART_Handler(void)
{
	Serial.IrqHandler();
}

/*****************
USART Objects
******************/

USARTClass Serial1(1);
USARTClass Serial2(2);
USARTClass Serial3(3);

//Interrupt Handlers
void USART0_Handler(void){
	Serial1.IrqHandler();
}

void USART1_Handler(void){	Serial2.IrqHandler();
}

void USART3_Handler(void){
	Serial3.IrqHandler();}


/*****************
I2C Objects
******************/

/** Class objects*/
TwoWireClass Wire(TWI1);	//20, 21
TwoWireClass Wire1(TWI0);	//Unlabeled

/** Interrupt Handlers*/
void TWI0_Handler(){ //Wire1
	Wire1.IRQHandler();
};

void TWI1_Handler(){ //Wire
	Wire.IRQHandler();
}

/*****************
SPI Objects
******************/
SPIClass SPI;

void SPI0_Handler(){
	SPI.IRQHandler();
}
/*****************
Timer Objects
******************/
SystemTickClass milliSysTick(0, 1000);

void TC0_Handler(){
	milliSysTick.IRQHandler();
}
