
#ifndef _USART_
#define _USART_

/************************************************************************/
/* Headers                                                              */
/************************************************************************/
#include "../libraries.h"

//Calculates clock divisor (CD) field for USART_BRGR
#define clockDivisor(baud) 84000000L/(16*baud)


class USARTClass : public RingBufferClass
{
public:
	USARTClass(uint32_t channel);
	void		begin();
	void		begin(uint32_t baudRate);
	void		begin(uint32_t baudRate, uint32_t mode);
	void		flushAll();
	uint32_t	write(uint32_t val, BitLength length = WORD);
	uint32_t	read(BitLength length = BYTE);
	
	void IrqHandler(void);
	
private:
	//Instance and pin definitions
	Usart *instance;
	IRQn_Type IRQNum;
	uint32_t instanceID, instanceMode;
	uint32_t rxPin, txPin, baud, initialized;
	
	//Ring Buffer Instantiation
	static const uint32_t MAXBUFFERSIZE = 150;
	uint32_t rxBuff[MAXBUFFERSIZE];
	uint32_t txBuff[MAXBUFFERSIZE];
	RingBufferClass rxBuffer = RingBufferClass(rxBuff, MAXBUFFERSIZE);
	RingBufferClass txBuffer = RingBufferClass(txBuff, MAXBUFFERSIZE);
	
	void init();
};


/************************************************************************/
/* Exported Functions                                                   */
/************************************************************************/
extern void USART_Configure(Usart *usartInstance, uint32_t periphID, IRQn_Type IRQID, uint32_t mode, uint32_t baudRate);


#endif /* #ifndef _USART_ */