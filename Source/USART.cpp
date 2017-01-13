//TX3 & RX3 are USART3 -> Serial3
//TX2 & RX2 are USART1 -> Serial2
//TX1 & RX1 are USART0 -> Serial1


#include "../libraries.h"

USARTClass::USARTClass(uint32_t channel){

	switch(channel)
	{
		case 1:
		instance = USART0;
		instanceID = ID_USART0;
		IRQNum = USART0_IRQn;
		txPin  = 18;
		rxPin  = 19;
		break;
		
		case 2:
		instance = USART1;
		instanceID = ID_USART1;
		IRQNum = USART1_IRQn;
		txPin  = 16;
		rxPin  = 17;
		break;
		
		case 3:
		instance = USART3;
		instanceID = ID_USART3;
		IRQNum = USART3_IRQn;
		txPin  = 14;
		rxPin  = 15;
		break;
		default: return;
	}
	
	initialized = false;
}

void USARTClass::begin(){
	baud = 115200;
	instanceMode = US_MR_USART_MODE_NORMAL | US_MR_USCLKS_MCK| US_MR_CHMODE_NORMAL | US_MR_CHRL_8_BIT|
					US_MR_PAR_NO;
	init();
}

void USARTClass::begin(uint32_t baudRate){
	baud = baudRate;
	instanceMode = US_MR_USART_MODE_NORMAL | US_MR_USCLKS_MCK| US_MR_CHMODE_NORMAL | US_MR_CHRL_8_BIT|
					US_MR_PAR_NO;
	init();
}

void USARTClass::begin(uint32_t baudRate, uint32_t mode){
	baud = baudRate;
	instanceMode = mode;
	init();
}

void USARTClass::init(){
	//Has the USART channel already been initialized?
	if(initialized)
	return;
	
	//Initialize pins and hand control over to peripheral.
	PIO_configurePin(
	pinCharacteristic[txPin].port,
	pinCharacteristic[txPin].pinMask,
	pinCharacteristic[txPin].peripheralType,
	pinCharacteristic[txPin].pinAttribute,
	OUTPUT);
	
	PIO_configurePin(
	pinCharacteristic[rxPin].port,
	pinCharacteristic[rxPin].pinMask,
	pinCharacteristic[rxPin].peripheralType,
	pinCharacteristic[rxPin].pinAttribute,
	INPUT);
	
	USART_Configure(instance, instanceID, IRQNum, instanceMode, baud);
	
	//Lock out this function from further use.
	initialized = true;
}

void USARTClass::flushAll(){
	txBuffer.flush();
	rxBuffer.flush();
}

//General write function that handles full 32bit values natively
uint32_t USARTClass::write(uint32_t val, BitLength length){
	//If TX hardware is busy with a write, buffer data
	if ((instance->US_CSR & US_CSR_TXRDY) != US_CSR_TXRDY){
		
		//Based on the length of data to send, mask off bits and shift right to form 8bit value
		switch (length){
			case BYTE:
			case _8BIT:
				txBuffer.write(0xFF & val);
			break;
			
			case HALFWORD:
			case _16BIT:
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
			break;
			
			case _24BIT:
				txBuffer.write((val >> 16) & 0xFF);
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
			break;
			
			case WORD:
			case _32BIT:
				txBuffer.write((val >> 24) & 0xFF);
				txBuffer.write((val >> 16) & 0xFF);
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
			break;
		}
		
		//Enable TX hardware ready interrupt to trigger sending of buffered data
		instance->US_IER = US_IER_TXRDY;
		
		return 1u;
		
	} else {
		switch (length){
			case BYTE: //Hardware not busy, so send immediately
			case _8BIT:
				instance->US_THR = (uint8_t)(val & 0xFF);
			break;
			
			case HALFWORD:
			case _16BIT:
				//Buffer last byte
				txBuffer.write(val & 0xFF);
				
				//Ensure interrupts are enabled
				instance->US_IER = US_IER_TXRDY;
				
				//Send the most significant byte
				instance->US_THR = (uint8_t)((val >> 8) & 0xFF);
			break;
			
			case _24BIT:
				//Buffer
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
				
				//Ensure interrupts are enabled
				instance->US_IER = US_IER_TXRDY;
				
				//Send the most significant byte
				instance->US_THR = (uint8_t)((val >> 16) & 0xFF);
			break;
			
			case WORD:
			case _32BIT:
				//Buffer data
				txBuffer.write((val >> 16) & 0xFF);
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
				
				//Ensure interrupts are enabled
				instance->US_IER = US_IER_TXRDY;
				
				//Send the upper 8 bits first
				instance->US_THR = (uint8_t)((val >> 24) & 0xFF);
			break;
		}
		
		return 1u;
	}
}

//General read function that can handle full 32bit values natively
uint32_t USARTClass::read(BitLength length){
	uint32_t bits31_24, bits23_16, bits15_8, bits7_0;
	
	//Based on the length of data to read, recombine info
	switch (length)
	{
		case BYTE:
		case _8BIT:
			return rxBuffer.read() & 0xFF;
			break;
		
		case HALFWORD:
		case _16BIT:
			bits15_8  = (rxBuffer.read() & 0xFF) << 8;
			bits7_0   = (rxBuffer.read() & 0xFF);
			
			return (bits15_8 | bits7_0);
			break;
		
		case _24BIT:
			bits23_16 = (rxBuffer.read() & 0xFF) << 16;
			bits15_8  = (rxBuffer.read() & 0xFF) << 8;
			bits7_0   = (rxBuffer.read() & 0xFF);
			
			return (bits23_16 | bits15_8 | bits7_0);
			break;
		
		case WORD:
		case _32BIT:
			bits31_24 = (rxBuffer.read() & 0xFF) << 24;
			bits23_16 = (rxBuffer.read() & 0xFF) << 16;
			bits15_8  = (rxBuffer.read() & 0xFF) << 8;
			bits7_0   = (rxBuffer.read() & 0xFF);
			
			return (bits31_24 | bits23_16 | bits15_8 | bits7_0);
			break;
		
		default: return 0u;
	}
}

void USARTClass::IrqHandler(void){
	uint32_t status = instance->US_CSR;
	
	// Did we receive data?
	if((status & US_CSR_RXRDY) == US_CSR_RXRDY)
		rxBuffer.write(instance->US_RHR); //8bit format

	// Do we need to keep sending data?
	if((status & US_CSR_TXRDY) == US_CSR_TXRDY)
	{
		if(txBuffer.availableToRead()){
			//Make sure interrupts are still enabled
			instance->US_IER |= US_IER_TXRDY;
			
			//Send the data
			instance->US_THR = (uint8_t)txBuffer.read();
		}
		else
			instance->US_IDR |= US_IDR_TXRDY; //Turn off interrupt. No more data left.
	}

	// Acknowledge errors
	if ((status & US_CSR_OVRE) == US_CSR_OVRE || (status & US_CSR_FRAME) == US_CSR_FRAME)
	{
		// TODO: error reporting outside ISR
		instance->US_CR |= US_CR_RSTSTA;
	}
}


/************************************************************************/
/* Exported Functions                                                   */
/************************************************************************/

void USART_Configure(Usart *usartInstance, uint32_t periphID, IRQn_Type IRQID, uint32_t mode, uint32_t baudRate){
	//Turn on peripheral clock
	pmc_enable_periph_clk(periphID);
	
	//Disable PDC Requests
	usartInstance->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;
	
	//Reset TX, RX & Status Register. Disable TX & RX
	usartInstance->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS | US_CR_RSTSTA;
	
	//Set the mode: Normal Channel, Master Clock, 8 Bit, and whatever is defined in usartMode
	usartInstance->US_MR = mode;
	
	//Set the Baud Rate
	usartInstance->US_BRGR = clockDivisor(baudRate);
	
	//Disable all interrupts and then config
	usartInstance->US_IDR = 0xFFFFFFFF;
	NVIC_DisableIRQ(IRQID);
	NVIC_ClearPendingIRQ(IRQID);
	NVIC_SetPriority(IRQID, (uint32_t)PRIOR_SERIAL);
	NVIC_EnableIRQ(IRQID);
	usartInstance->US_IER = US_IER_RXRDY;
	
	//Enable TX & RX
	usartInstance->US_CR &= ~(US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS | US_CR_RSTSTA);
	usartInstance->US_CR = US_CR_RXEN | US_CR_TXEN;
}