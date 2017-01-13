/*
 * UART.cpp
 *
 * Created: 12/13/2015 12:22:22 PM
 *  Author: Brandon
 */ 

#include "../libraries.h"

//Constructor
UARTClass::UARTClass(){
}

//Initializes UART to default parameters
void UARTClass::begin(){
	//Enable the peripheral clock to the UART
	pmc_enable_periph_clk(ID_UART);
	
	/** Configure RX0/TX0 through PIO. This also disables PIO control
	of the pins and hands it over to the peripheral controller.*/
	PIO_configurePin(
	pinCharacteristic[RX0].port, 
	pinCharacteristic[RX0].pinMask, 
	pinCharacteristic[RX0].peripheralType,
	pinCharacteristic[RX0].pinAttribute, INPUT);
	
	PIO_configurePin(
	pinCharacteristic[TX0].port,
	pinCharacteristic[TX0].pinMask,
	pinCharacteristic[TX0].peripheralType,
	pinCharacteristic[TX0].pinAttribute, OUTPUT);
	
	// Disable PDC channel requests
	UART->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;
	
	// Reset and disable transmitter and receiver
	UART->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS;
	
	//Set the mode: Normal Channel
	UART->UART_MR = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO;
	
	//Set the baud rate: 115200
	UART->UART_BRGR = clockDivisor(115200);
	
	//Disable all interrupts and then configure for the one you want
	UART->UART_IDR = 0xFFFFFFFF; //Disable all
	NVIC_EnableIRQ(UART_IRQn);
	UART->UART_IER = UART_IER_RXRDY; 
	
	// Enable receiver and transmitter
	UART->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
}

//Initializes UART to default parameters + baudrate
void UARTClass::begin(uint32_t baudRate){
	/** Configure RX0/TX0 through PIO. This also disables PIO control
	of the pins and hands it over to the peripheral controller.*/
	PIO_configurePin(
	pinCharacteristic[RX0].port, 
	pinCharacteristic[RX0].pinMask, 
	pinCharacteristic[RX0].peripheralType,
	pinCharacteristic[RX0].pinAttribute, INPUT);
	
	PIO_configurePin(
	pinCharacteristic[TX0].port,
	pinCharacteristic[TX0].pinMask,
	pinCharacteristic[TX0].peripheralType,
	pinCharacteristic[TX0].pinAttribute, OUTPUT);
	
	//Enable the peripheral clock to the UART
	pmc_enable_periph_clk(ID_UART);
	
	//Disable PDC channel requests
	UART->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;
	
	//Reset and disable transmitter and receiver
	UART->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS;
	
	//Set the mode: Normal Channel
	UART->UART_MR = UART_MR_CHMODE_NORMAL;
	
	//Set the baud rate: 115200
	UART->UART_BRGR = clockDivisor(baudRate);
	
	//Disable all interrupts and then configure for the one you want
	UART->UART_IDR = 0xFFFFFFFF; //Disable all
	NVIC_EnableIRQ(UART_IRQn);
	UART->UART_IER = UART_IER_RXRDY;
	
	//Enable receiver and transmitter
	UART->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
}

//Initializes UART to "mode" parameters + baudrate
void UARTClass::begin(uint32_t baudRate, uint32_t mode){
	/** Configure RX0/TX0 through PIO. This also disables PIO control
	of the pins and hands it over to the peripheral controller.*/
	PIO_configurePin(
	pinCharacteristic[RX0].port, 
	pinCharacteristic[RX0].pinMask, 
	pinCharacteristic[RX0].peripheralType,
	pinCharacteristic[RX0].pinAttribute, INPUT);
	
	PIO_configurePin(
	pinCharacteristic[TX0].port,
	pinCharacteristic[TX0].pinMask,
	pinCharacteristic[TX0].peripheralType,
	pinCharacteristic[TX0].pinAttribute, OUTPUT);
	
	//Enable the peripheral clock to the UART
	pmc_enable_periph_clk(ID_UART);
	
	// Disable PDC channel requests
	UART->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;
	
	// Reset and disable transmitter and receiver
	UART->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS;
	
	//Set the mode: Even parity and Normal Channel
	UART->UART_MR = mode;
	
	//Set the baud rate: 115200
	UART->UART_BRGR = clockDivisor(baudRate);
	
	//Disable all interrupts and then configure for the one you want
	UART->UART_IDR = 0xFFFFFFFF; //Disable all
	NVIC_DisableIRQ(UART_IRQn);
	NVIC_ClearPendingIRQ(UART_IRQn);
	NVIC_SetPriority(UART_IRQn, (uint32_t)PRIOR_SERIAL);
	NVIC_EnableIRQ(UART_IRQn);
	UART->UART_IER = UART_IER_RXRDY; 
	
	// Enable receiver and transmitter
	UART->UART_CR = UART_CR_RXEN | UART_CR_TXEN;
}

//Clear the TX and RX buffers
void UARTClass::flushAll(){
	txBuffer.flush();
	rxBuffer.flush();
}

//More general write function that handles full 32bit values
uint32_t UARTClass::write(uint32_t val, BitLength length){
	//If TX hardware is busy with a write, buffer data
	if ((UART->UART_SR & UART_SR_TXRDY) != UART_SR_TXRDY){
		
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
		UART->UART_IER = UART_IER_TXRDY;
		
		return 1u;
		
	} else {
		switch (length){
			case BYTE: //Hardware not busy, so send immediately
			case _8BIT:
				UART->UART_THR = (uint8_t)(val & 0xFF);
			break;
			
			case HALFWORD:
			case _16BIT:
				//Buffer last byte
				txBuffer.write(val & 0xFF);
				
				//Ensure interrupts are enabled
				UART->UART_IER = UART_IER_TXRDY;
				
				//Send the most significant byte
				UART->UART_THR = (uint8_t)((val >> 8) & 0xFF);
			break;
			
			case _24BIT:
				//Buffer
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
				
				//Ensure interrupts are enabled 
				UART->UART_IER = UART_IER_TXRDY;
				
				//Send most significant byte
				UART->UART_THR = (uint8_t)((val >> 16) & 0xFF);
				break;
			
			case WORD:
			case _32BIT:
				//Buffer data
				txBuffer.write((val >> 16) & 0xFF);
				txBuffer.write((val >> 8) & 0xFF);
				txBuffer.write(val & 0xFF);
				
				//Ensure interrupts are enabled
				UART->UART_IER = UART_IER_TXRDY;
				
				//Send upper 8 bits first
				UART->UART_THR = (uint8_t)((val >> 24) & 0xFF);
			break;
		}
		
		return 1u;
	}
}

//Reads a single byte
uint32_t UARTClass::read(BitLength length){
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
 

void UARTClass::IrqHandler(void){
	uint32_t status = UART->UART_SR;
	
	//Did we receive data?
	if ((status & UART_SR_RXRDY) == UART_SR_RXRDY)
		rxBuffer.write(UART->UART_RHR);

	//Do we need to keep sending data?
	if ((status & UART_SR_TXRDY) == UART_SR_TXRDY){
		//Is data available to send?
		if (txBuffer.availableToRead())
			UART->UART_THR = (uint8_t)txBuffer.read();
		else
			UART->UART_IDR = UART_IDR_TXRDY;  //Turn off interrupt. No more data left.
	}

	// Acknowledge errors
	if ((status & UART_SR_OVRE) == UART_SR_OVRE || (status & UART_SR_FRAME) == UART_SR_FRAME){
		// TODO: error reporting outside ISR
		UART->UART_CR |= UART_CR_RSTSTA;
	}
}