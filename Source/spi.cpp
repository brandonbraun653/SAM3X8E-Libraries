/*
 * spi.cpp
 *
 * Created: 1/1/2017 8:15:35 PM
 *  Author: Brandon
 */ 

#include "../libraries.h"

SPIClass::SPIClass(){
	spi = SPI0;
	IRQnID = SPI0_IRQn;
	periphID = ID_SPI0;
}

/************************************************************************/
/* User level SPI Functions                                             */
/************************************************************************/

/** Starts SPI as a Master device **/
void SPIClass::begin(){
	//If SPI hardware has already been initialized, exit.
	if(initialized)
		return;
	
	//Initialize hardware
	init_SPI_MASTER();
	
	//Clear the receive buffer
	rxBuffer.flush();
	
	//Start the random number generator
	RNG_start();
	
	hardwareMode = MASTER;
	lastChannelNum = 0;
	initialized = true;
}

void SPIClass::attachPinToChannel(uint32_t channel, uint32_t pin){
	settings[channel].DIGPIN = pin;
	settings[channel].PORT = lookUp_Port(pin);
	settings[channel].MASK = lookUp_Mask(pin);
	
	//Setup the pin properly
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
}

void SPIClass::configureChannel(uint32_t channel, uint32_t config, uint32_t freq){
	//Mask off configuration bits and store them
	settings[channel].CPOL = config & (1u<<0);
	settings[channel].NCPHA = config & (1u<<1);
	settings[channel].BITS = config & (0xFu<<4);
	settings[channel].FREQ = SPI_CLOCK_FREQ(freq);
}

void SPIClass::applyChannelConfig(uint32_t channel, SPIHW hardwareChannel){
	uint32_t config =
		settings[channel].CPOL |
		settings[channel].NCPHA |
		settings[channel].BITS |
		settings[channel].FREQ;
	
	//Wait until the transmission is complete if there is one
	while ((spi->SPI_SR & SPI_SR_TXEMPTY) == 0);
	
	if(hardwareChannel == RANDOM){
		uint32_t kickedOffChannel = RNG_generate() % MAX_HW_CHANNEL;
		SPI_ConfigureNPCS(spi, kickedOffChannel, config);
		
		//Save channel state
		settings[channel].attachedChannel = kickedOffChannel;
		settings[channel].attached = true;
		return;
	} else {
		SPI_ConfigureNPCS(spi, (uint32_t)hardwareChannel, config);
		
		//Save channel state
		settings[channel].attachedChannel = (uint32_t)hardwareChannel;
		settings[channel].attached = true;
		return;
	}
}

void SPIClass::write(uint32_t channel, uint16_t data, bool CSNAAT){
	if(channel > MAX_SW_CHANNELS)
		return;
	
	/** Check for channel continuity **/
	if(lastChannelNum != channel){
		//Shoot. Disable the last channel's CS pin
		settings[lastChannelNum].PORT->PIO_SODR |= settings[lastChannelNum].MASK;
		
		//Update to the new channel
		lastChannelNum = channel;
		
		//Since it is a new channel, check for applied channel hardware settings
		if(settings[channel].attached == false){
			applyChannelConfig(channel, RANDOM);
		}
	}
	
	
	//Write CS pin low
	settings[channel].PORT->PIO_CODR |= settings[channel].MASK;
		
	//Send data from correct channel settings
	spi->SPI_TDR = data | SPI_PCS(settings[channel].attachedChannel);
		
	/** Wait until transfer is done, then deactivate CS (write HIGH) if needed.
		This blocking wait method is significantly faster than interrupts 
		at most used SPI clock frequencies, primarily due to ISR overhead.
		ISR jump in:  358 MASTER_CLOCK
		ISR jump out: 128 MASTER_CLOCK
	**/
	while((spi->SPI_SR & SPI_SR_RDRF) == 0);
		
	if(CSNAAT == true)
		settings[channel].PORT->PIO_SODR |= settings[channel].MASK;
		
	//Read in the data from last transfer and mask receive channel
	uint32_t receivedData = spi->SPI_RDR & 0xFFFF;
	uint32_t maskedChannel = (channel << 16) & 0xFFFF0000;
	rxBuffer.write(maskedChannel | receivedData);
}

uint32_t SPIClass::read(){
	return rxBuffer.read();
}

void SPIClass::end(){
	SPI_Disable(SPI0);
	initialized = false; 
}

void SPIClass::IRQHandler(){
	//Used in SLAVE mode. Currently not supported.
}

/************************************************************************/
/* Private Functions                                                    */
/************************************************************************/
	
void SPIClass::init_SPI_MASTER(){
	//Enable peripheral clock for SPI
	pmc_enable_periph_clk(periphID);
	
	//Configure MISO
	PIO_configurePin(
	pinCharacteristic[MISO].port,
	pinCharacteristic[MISO].pinMask,
	pinCharacteristic[MISO].peripheralType,
	pinCharacteristic[MISO].pinAttribute,
	INPUT);
	
	//Configure MOSI
	PIO_configurePin(
	pinCharacteristic[MOSI].port,
	pinCharacteristic[MOSI].pinMask,
	pinCharacteristic[MOSI].peripheralType,
	pinCharacteristic[MOSI].pinAttribute,
	OUTPUT);
	
	//Configure SCK
	PIO_configurePin(
	pinCharacteristic[SCK].port,
	pinCharacteristic[SCK].pinMask,
	pinCharacteristic[SCK].peripheralType,
	pinCharacteristic[SCK].pinAttribute,
	OUTPUT);
	
	//Initialize all chip select configurations to default
	for(uint32_t x=0; x<MAX_SW_CHANNELS; x++){
		settings[x].CPOL = 0u<<0;					//Clock inactive state = LOW
		settings[x].NCPHA = 0u<<1;					//Data changed Low->High, captured High->Low
		settings[x].BITS = 0xFu<<4;					//8-Bit Mode
		settings[x].FREQ = SPI_CLOCK_FREQ(1000000); //4MHz Clock
	}
	
	//Default configuration
	SPI_Configure(spi, periphID, (SPI_MR_MSTR | SPI_MR_MODFDIS | SPI_MR_PS | SPI_MR_WDRBT));
	
	//Interrupt setup
	NVIC_DisableIRQ(IRQnID);
	NVIC_ClearPendingIRQ(IRQnID);
	NVIC_SetPriority(IRQnID, (uint32_t)PRIOR_SPI);
	NVIC_EnableIRQ(IRQnID);
	
	//Enable
	SPI_Enable(spi);
}


/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/


extern void SPI_Enable( Spi* spi ){
    spi->SPI_CR = SPI_CR_SPIEN ;
}

extern void SPI_Disable( Spi* spi ){
    spi->SPI_CR = SPI_CR_SPIDIS ;
}

extern void SPI_EnableIt( Spi* spi, uint32_t dwSources ){
    spi->SPI_IER = dwSources ;
}

extern void SPI_DisableIt( Spi* spi, uint32_t dwSources ){
    spi->SPI_IDR = dwSources ;
}

extern void SPI_Configure( Spi* spi, uint32_t id, uint32_t configuration ){
    //Enable clock for the SPI0 peripheral
    pmc_enable_periph_clk(id);
	
	//Disable SPI instance so it can be reconfigured
	SPI_Disable(spi);
    
	/* Execute a software reset of the SPI twice */
    spi->SPI_CR = SPI_CR_SWRST;
    spi->SPI_CR = SPI_CR_SWRST;
    spi->SPI_MR = configuration;
}

extern void SPI_ConfigureNPCS( Spi* spi, uint32_t npcs, uint32_t configuration ){
    spi->SPI_CSR[npcs] = configuration ;
}

extern uint32_t SPI_GetStatus( Spi* spi ){
    return spi->SPI_SR ;
}

extern uint32_t SPI_Read( Spi* spi )
{
    while ( (spi->SPI_SR & SPI_SR_RDRF) == 0 ) ;

    return spi->SPI_RDR & 0xFFFF ;
}

extern void SPI_Write( Spi* spi, uint32_t npcs, uint16_t data ){
    /* Send data */
    while ( (spi->SPI_SR & SPI_SR_TXEMPTY) == 0 ) ;
    spi->SPI_TDR = data | SPI_PCS(npcs) ;
    while ( (spi->SPI_SR & SPI_SR_TDRE) == 0 ) ;
}

extern uint32_t SPI_IsFinished( Spi* spi ){
    return ((spi->SPI_SR & SPI_SR_TXEMPTY) != 0) ;
}