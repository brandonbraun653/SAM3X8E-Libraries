/*
 * UART.h
 *
 * Created: 12/13/2015 12:22:03 PM
 *  Author: Brandon
 */ 

/*
	This library controls the UART of the SAM3X8E MCU. On the UDOO board,
	these are digital pins 0 and 1, which correspond to RX0 and TX0 respectively.
	
	The other TX/RX lines on the board are controlled by the USART library.
*/

#ifndef UART_H_
#define UART_H_

#include "../libraries.h"

//Calculates clock divisor (CD) field for UART_BRGR
#define clockDivisor(baud) 84000000L/(16*baud)

class UARTClass : public RingBufferClass
{
public:
	UARTClass();

	void		begin();
	void		begin(uint32_t baudRate);
	void		begin(uint32_t baudRate, uint32_t mode);
	void		flushAll();
	uint32_t	write(uint32_t val, BitLength length = WORD);
	uint32_t	read(BitLength length = BYTE);
	
	void IrqHandler(void);
	
protected:
private:
	//Ring Buffer Instantiation
	uint32_t rxBuff[16];
	uint32_t txBuff[16];
	RingBufferClass rxBuffer = RingBufferClass(rxBuff, 16);
	RingBufferClass txBuffer = RingBufferClass(txBuff, 16);
};

#endif /* UART_H_ */