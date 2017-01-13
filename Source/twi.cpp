/*
 * twi.cpp
 *
 * Created: 11/23/2015 8:16:23 PM
 *  Author: Brandon
 */ 

/*
Design Notes:
	1) Use beginTransmission to set master into send mode. Any writes will be buffered up to 128 bytes
	   in the txBuffer array. Ending the transmission triggers a write of all data to the slave.
*/


#include "../libraries.h"



/************************************************************************/
/* Class Definition                                                     */
/************************************************************************/

/** Constructor*/
TwoWireClass::TwoWireClass(Twi *pTwi){
	//Clear buffers
	txBuffer.flush();
	rxBuffer.flush();
	svBuffer.flush();
	
	//Save pin numbers
	if(pTwi == TWI1){ //Wire
		SDA = 20u;
		SCL = 21u;
		twi = pTwi;
		periphID = ID_TWI1;
		IRQnID = TWI1_IRQn;
		twiClock = 400000;
		
		//Ensure that peripheral clock for SDA/SCL PIO controller is on
		pmc_enable_periph_clk(ID_PIOB);
	} else {		 //Wire1
		SDA = 70u;
		SCL = 71u;
		twi = pTwi;
		periphID = ID_TWI0;
		IRQnID = TWI0_IRQn;
		twiClock = 400000;
		
		//Ensure that peripheral clock for SDA/SCL PIO controller is on
		pmc_enable_periph_clk(ID_PIOA);
	}
	
	//Setup the PIO pins correctly
	setupHardware();
	
	//Enable I2C peripheral clock
	pmc_enable_periph_clk(periphID);
	
	//Interrupt handling
	NVIC_DisableIRQ(IRQnID);
	NVIC_ClearPendingIRQ(IRQnID);
	NVIC_SetPriority(IRQnID, (uint32_t)PRIOR_I2C);
	NVIC_EnableIRQ(IRQnID);
	
	TWISTATUS = UNINITIALIZED;
}

/** Initializes as I2C Master*/
uint32_t TwoWireClass::begin(){
	if(initialized == true)
		return 1u;
		
	TWISTATUS = MASTER_IDLE;
	TWIMESSAGE = NO_MESSAGE;
	TWIERROR = NO_ERROR;
	initialized = true;
	
	return TWI_ConfigureMaster(twi, twiClock, MASTER_CLOCK);
}

/** Initializes as I2C Slave with slave address*/
void TwoWireClass::begin(uint8_t slaveAddress){
	//setupHardware();
	
	TWI_ConfigureSlave(twi, slaveAddress);
	TWI_EnableIt(twi, TWI_IER_SVACC);

	TWISTATUS = SLAVE_IDLE;
}

/** Prepares software for transmitting*/
void TwoWireClass::beginTransmission(uint8_t slaveAddress){
	TWISTATUS = MASTER_SEND;
	TWIMESSAGE = TRANSMIT_START;
	
	Packet.chip = slaveAddress;
	Packet.iaddr[0] = 0u;
	Packet.iaddr[1] = 0u;
	Packet.iaddr[2] = 0u;
	Packet.iaddr_length = 0u;
	Packet.rxLength = 0u;
}

/** Writes a single byte into txBuffer or svBuffer **/
uint32_t TwoWireClass::write(uint32_t val){
	if(TWISTATUS == MASTER_SEND)
		return txBuffer.write(val);
	else
		return svBuffer.write(val);
}

/** Writes multiple bytes into txBuffer or svBuffer**/
uint32_t TwoWireClass::write(uint32_t *dataArray, uint32_t numToWrite){
	if(TWISTATUS == MASTER_SEND)
		return txBuffer.write(dataArray, numToWrite);
	else
		return svBuffer.write(dataArray, numToWrite);
}

/** Reads byte from rxBuffer*/
uint8_t TwoWireClass::read(){
	return rxBuffer.read();
}

/** Empties the transmit buffer by writing to slave device*/
void TwoWireClass::endTransmission(){
	TWIMESSAGE = TRANSMIT_ADDRESS;
	
	TWI_StartWrite(); //Send the first byte off and let ISR handle rest of transmission
}

uint32_t TwoWireClass::TWI_StartWrite(){
	bool iFlag = false; //Interrupt flag 
	uint32_t timeout = 0;
	
	//Make certain that all previous read and write operations are finished
	while((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP){
		++timeout;
		
		if(timeout > XMIT_TIMEOUT)
			return 0u;
	}
	
	/* Dummy read to clear bits*/
	twi->TWI_SR;
	
	/* Set write mode, slave address, and internal address length */
	twi->TWI_MMR = 0;
	twi->TWI_MMR = TWI_MMR_DADR(Packet.chip) |
	((Packet.iaddr_length << TWI_MMR_IADRSZ_Pos) &
	TWI_MMR_IADRSZ_Msk);
	
	/* Set internal address for remote chip */
	twi->TWI_IADR = 0;
	twi->TWI_IADR = TWI_makeAddr(Packet.iaddr, Packet.iaddr_length);
	
	/* Is this the only byte we are sending? */
	if(txBuffer.availableToRead() == 1u){
		Packet.registerAccess = txBuffer.peek(); //Grab the register being accessed inside the slave
		twi->TWI_CR |= TWI_CR_STOP;				 //Enable the stop bit at end of transmission
		iFlag = true;							 //Select the proper interrupts to fire
	}
	
	/* Enable start bit */
	twi->TWI_CR |= TWI_CR_START;
	
	/* Write first byte to send.*/
	twi->TWI_THR = txBuffer.read();
	
	//Enable interrupts. This must be last or interrupts will be triggered immediately
	if(iFlag)
		TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_TXCOMP); //Last transmission
	else
		TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_TXRDY);  //More to go
	return 1u;
}

void TwoWireClass::requestFrom(uint8_t slaveAddress, uint8_t quantity){
	TWISTATUS = MASTER_RECV;
	TWIMESSAGE = RECEIVE_START;
	
	Packet.chip = slaveAddress;
	Packet.iaddr[0] = 0u;
	Packet.iaddr[1] = 0u;
	Packet.iaddr[2] = 0u;
	Packet.iaddr_length = 0u;
	Packet.rxLength = quantity;
	leftToRead = quantity;
	
	//^^^^ Need to some how flag this to be non-blocking. Is it worth it to 
	//     get the avg of ~40uS in transmit time?
	
	TWI_StartRead();
	
	//Also blocking....*sigh...fix this kludge code tomorrow.
	//Can't use a global flag that is modified in interrupt. Colliding access is possible.
	//while(TWIMESSAGE != RECEIVE_SUCCESSFUL);
	
}

uint32_t TwoWireClass::TWI_StartRead(){
	TWIMESSAGE = TRANSMIT_ADDRESS;
	uint32_t timeout = 0;
	
	//Make sure all transmits have been completed before reading from slave.
	//MAJOR SCREWUPS otherwise. Complete bus failure.
	while((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP){
		++timeout;
		
		if(timeout > XMIT_TIMEOUT)
		return 0u;
	};
	
	/* Dummy read to clear bits*/
	twi->TWI_SR;
	
	/* Set read mode, slave address, and internal address length */
	twi->TWI_MMR = 0;
	twi->TWI_MMR = TWI_MMR_DADR(Packet.chip) | TWI_MMR_MREAD | 
	((Packet.iaddr_length << TWI_MMR_IADRSZ_Pos) &
	TWI_MMR_IADRSZ_Msk);
	
	
	/* Set internal address for remote chip */
	twi->TWI_IADR = 0;
	twi->TWI_IADR = TWI_makeAddr(Packet.iaddr, Packet.iaddr_length);
	
	
	/* Do we only need 1 byte from the slave? */
	if(Packet.rxLength == 1){
		//Start and stop bits must be set at same time. This begins transmission of slave addr.
		twi->TWI_CR &= ~(TWI_CR_START | TWI_CR_STOP);
		twi->TWI_CR |= TWI_CR_START | TWI_CR_STOP; //pg.715
		
		//Set interrupts and let ISR handler take care of the rest.
		TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_RXRDY);
	} else {
		//Set only the start bit to begin slave addr transmission
		twi->TWI_CR &= ~(TWI_CR_START);
		twi->TWI_CR |= TWI_CR_START;
		
		//Set interrupts and let ISR handler take care of the rest.
		TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_RXRDY);
	}
	return 1u; //Success
}

/** Interrupt Handler*/
void TwoWireClass::IRQHandler(){
	//Read status register
	uint32_t status = twi->TWI_SR;
	
	//Disable interrupts so we don't accidentally loop upon next transmit
	TWI_DisableIt(twi, TWI_IDR_NACK | TWI_IDR_TXRDY | TWI_IDR_TXCOMP | TWI_IDR_RXRDY);
	
	//Dummy indicator to make sure we made it here
	digitalWrite(25, HIGH);
	digitalWrite(25, LOW);
	
	if(TWISTATUS == MASTER_SEND){
		//No ACK received from slave? Figure out when it occurred.
		if((status & TWI_SR_NACK) == TWI_SR_NACK){
		
			if(TWIMESSAGE == TRANSMIT_ADDRESS){
				TWIERROR = ERROR_NACK_ON_ADDRESS_TRANSMIT;
			
				//To Do: Options for retry and timeout
				//			1) Number of retries or time? Probably should use Scheduler.
				//			2) Upon max failures: clear transmit buffer, alert main program of problem
			}
		
			if(TWIMESSAGE == TRANSMIT_DATA){
			
				//At this point, the data buffer has been read from and all
				//past data cannot be recovered. Purge buffer and alert main program
				//to retry. This could probably be improved upon.
				txBuffer.flush();
				TWIERROR = ERROR_NACK_DURING_TRANSMIT;
			}
			return;
		}
	
		//Error Free! Continue sending data as needed.
		uint32_t dataLeft = txBuffer.availableToRead();
		bool iFlag = false;
	
		if(dataLeft > 0){
			//Update the status
			TWIMESSAGE = TRANSMIT_DATA;
		
			//Is this the last byte of data to send?
			if(dataLeft == 1){
				twi->TWI_CR |= TWI_CR_STOP;	//Enable the stop bit at end of transmission
				iFlag = true;				//Set "finishing" interrupts to fire
			}
		
			//Write data
			twi->TWI_THR = txBuffer.read();
		
			//Enable interrupts. This must be last or interrupts will be triggered immediately
			if(iFlag)
				TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_TXCOMP); //Last transmission
			else
				TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_TXRDY);  //More data to go
			return;
		}
	
	
		//Data transmit completed with no errors. Tidy up transmission.
		if((status & TWI_SR_TXCOMP) == TWI_SR_TXCOMP){
			TWISTATUS = MASTER_IDLE;
			TWIERROR = NO_ERROR;
			TWIMESSAGE = TRANSMIT_SUCCESSFUL;
			
			//Clean buffer
			txBuffer.flush();
			
			//Make sure interrupts are disabled
			TWI_DisableIt(twi, TWI_IDR_NACK | TWI_IDR_TXRDY | TWI_IDR_TXCOMP);
			return;
		}
	} //End of TWISTATUS == MASTER_SEND
	
	
	if(TWISTATUS == MASTER_RECV){
		
		//No ACK received from slave? Figure out when it occurred.
		if((status & TWI_SR_NACK) == TWI_SR_NACK){
			
			if(TWIMESSAGE == TRANSMIT_ADDRESS){
				TWIERROR = ERROR_NACK_ON_ADDRESS_TRANSMIT;
				
				//To Do: Options for retry and timeout
				//			1) Number of retries or time? Probably should use Scheduler.
				//			2) Upon max failures: clear transmit buffer, alert main program of problem
			}
			
			
			if(TWIMESSAGE == RECEIVE_DATA){
				//Can we even have a NACK on our side???
			}
			return;
		}
		
		//No errors! We may proceed with reading data
		if(leftToRead > 0){
			TWIMESSAGE = RECEIVE_DATA;
			
			//Enable stop bit if next to last receive
			if(leftToRead == 1)
				twi->TWI_CR |= TWI_CR_STOP;
			
			//Read data from slave
			rxBuffer.write(twi->TWI_RHR);
			--leftToRead;	
				
				
			//Ensure interrupts are enabled
			TWI_EnableIt(twi, TWI_IER_NACK | TWI_IER_RXRDY);
		}
		
		//Looks like we're done here
		if(leftToRead == 0){
			TWIMESSAGE = RECEIVE_SUCCESSFUL;
			
			//Ensure interrupts are disabled
			TWI_DisableIt(twi, TWI_IDR_NACK | TWI_IDR_RXRDY);
			return;
		}
		
		
	} //End of TWISTATUS == MASTER_RECV

	
}

/** Returns how many bytes are available to read from the rxBuffer*/
uint8_t TwoWireClass::availableToRead(){
	return rxBuffer.availableToRead();
}

/** Assigns user defined receive event function*/
void TwoWireClass::onReceive(void(*function)(int)){
	onReceiveCallback = function;
}

/** Assigns user defined request event function*/
void TwoWireClass::onRequest(void(*function)(void)){
	onRequestCallback = function;
}

/** Initializes I2C hardware*/
void TwoWireClass::setupHardware(){

	PIO_configurePin(
	pinCharacteristic[SDA].port,
	pinCharacteristic[SDA].pinMask,
	pinCharacteristic[SDA].peripheralType,
	pinCharacteristic[SDA].pinAttribute, 0x20);
	
	PIO_configurePin(
	pinCharacteristic[SCL].port,
	pinCharacteristic[SCL].pinMask,
	pinCharacteristic[SCL].peripheralType,
	pinCharacteristic[SCL].pinAttribute, 0x20);
}




/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Configures a TWI peripheral to operate in master mode, at the given
 * frequency (in Hz). The duty cycle of the TWI clock is set to 50%.
 * \param pTwi  Pointer to an Twi instance.
 * \param twck  Desired TWI clock frequency.
 * \param mck  Master clock frequency.
 */
uint32_t TWI_ConfigureMaster( Twi* pTwi, uint32_t dwTwCk, uint32_t dwMCk )
{
	/* Disable TWI interrupts */
	pTwi->TWI_IDR = ~0UL;

	/* Dummy read in status register */
	pTwi->TWI_SR;
	
	//Disable all PDC channel requests
	//pTwi->TWI_PTCR = TWI_PTCR_RXTDIS | TWI_PTCR_TXTDIS;
	
	/* Reset the TWI */
	TWI_Reset(pTwi);
	
	/* Set as Master */
	TWI_SetMaster(pTwi);
	
	/* Configure clock*/
	if(TWI_SetClock(pTwi, dwTwCk, dwMCk) == 0u) 
		return 0u;
	
	return 1u;
}

void TWI_SetMaster(Twi *pTwi){
	/* Set Master Disable bit and Slave Disable bit */
	pTwi->TWI_CR = TWI_CR_MSDIS;
	pTwi->TWI_CR = TWI_CR_SVDIS;

	/* Set Master Enable bit */
	pTwi->TWI_CR = TWI_CR_MSEN;
	
	//Dummy read on status register to clear TXRDY
	pTwi->TWI_SR;
}

uint32_t TWI_SetClock( Twi *pTwi, uint32_t dwTwCk, uint32_t dwMCk )
{	
	uint32_t ckdiv = 0;
	uint32_t c_lh_div;

	if (dwTwCk > 400000) {
		return 0u;
	}

	c_lh_div = dwMCk / (dwTwCk * 2) - 4;

	/* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
	while ((c_lh_div > 0xFF) && (ckdiv < 7)) {
		/* Increase clock divider */
		ckdiv++;
		/* Divide cldiv value */
		c_lh_div /= 2;
	}

	/* set clock waveform generator register */
	pTwi->TWI_CWGR =
	TWI_CWGR_CLDIV(c_lh_div) | TWI_CWGR_CHDIV(c_lh_div) |
	TWI_CWGR_CKDIV(ckdiv);
	
	return 1u;
}

/**
 * \brief Configures a TWI peripheral to operate in slave mode.
 * \param pTwi  Pointer to an Twi instance.
 * \param slaveAddress Slave address.
 */
void TWI_ConfigureSlave(Twi *pTwi, uint8_t slaveAddress)
{
    //uint32_t i;

    /* TWI software reset */
    pTwi->TWI_CR = TWI_CR_SWRST;
    pTwi->TWI_RHR;

    /* Wait at least 10 ms */
    //for (i=0; i < 1000000; i++);

	/* Configure slave address. */
	pTwi->TWI_SMR = 0;
	pTwi->TWI_SMR = TWI_SMR_SADR(slaveAddress);

    /* TWI Slave Mode Disabled, TWI Master Mode Disabled*/
    pTwi->TWI_CR = TWI_CR_SVDIS | TWI_CR_MSDIS;

	/* Clear the SVDIS bit so the slave can be enabled: pg.737*/
	pTwi->TWI_CR = 0;

    /* SVEN: TWI Slave Mode Enabled */
    pTwi->TWI_CR = TWI_CR_SVEN;

    /* Wait at least 10 ms */
    //for (i=0; i < 1000000; i++);
}

/**
 * \brief Sends a START condition on the TWI.
 * \param pTwi  Pointer to an Twi instance.
 */
void TWI_Start(Twi *pTwi){
	pTwi->TWI_CR |= TWI_CR_START;
}

/**
 * \brief Sends a STOP condition on the TWI.
 * \param pTwi  Pointer to an Twi instance.
 */
void TWI_Stop( Twi *pTwi )
{
    pTwi->TWI_CR = TWI_CR_STOP;
}

void TWI_Reset(Twi *pTwi){
	/* Set SWRST bit to reset TWI peripheral */
	pTwi->TWI_CR = TWI_CR_SWRST;
	pTwi->TWI_RHR;
}

/**
 * \brief Starts a read operation on the TWI bus with the specified slave, it returns
 * immediately. Data must then be read using TWI_ReadByte() whenever a byte is
 * available (poll using TWI_ByteReceived()).
 * \param pTwi  Pointer to an Twi instance.
 * \param address  Slave address on the bus.
 * \param iaddress  Optional internal address bytes.
 * \param isize  Number of internal address bytes.
 */
void TWI_StartRead(
    Twi *pTwi,
    uint8_t slaveAddress,
    uint32_t iaddress,
    uint8_t isize)
{
  
    /* Set slave address and number of internal address bytes. */
    pTwi->TWI_MMR = 0;
    pTwi->TWI_MMR = (isize << 8) | TWI_MMR_MREAD | (slaveAddress << 16);

    /* Set internal address bytes */
    pTwi->TWI_IADR = 0;
    pTwi->TWI_IADR = iaddress;

    /* Send START condition */
    pTwi->TWI_CR |= TWI_CR_START;
}

/**
 * \brief Reads a byte from the TWI bus. The read operation must have been started
 * using TWI_StartRead() and a byte must be available (check with TWI_ByteReceived()).
 * \param pTwi  Pointer to an Twi instance.
 * \return byte read.
 */
uint8_t TWI_ReadByte(Twi *pTwi)
{
    return pTwi->TWI_RHR;
}

/**
 * \brief Sends a byte of data to one of the TWI slaves on the bus.
 * \note This function must be called once before TWI_StartWrite() with
 * the first byte of data  to send, then it shall be called repeatedly
 * after that to send the remaining bytes.
 * \param pTwi  Pointer to an Twi instance.
 * \param byte  Byte to send.
 */
void TWI_WriteByte(Twi *pTwi, uint8_t byte)
{
    pTwi->TWI_THR = byte;
}


uint32_t TWI_makeAddr(const uint8_t *addr, int len)
{
	uint32_t val;

	if (len == 0)
	return 0;

	val = addr[0];
	if (len > 1) {
		val <<= 8;
		val |= addr[1];
	}
	if (len > 2) {
		val <<= 8;
		val |= addr[2];
	}
	return val;
}

/**
 * \brief Check if a byte have been received from TWI.
 * \param pTwi  Pointer to an Twi instance.
 * \return 1 if a byte has been received and can be read on the given TWI
 * peripheral; otherwise, returns 0. This function resets the status register.
 */
uint8_t TWI_ByteReceived(Twi *pTwi)
{
    return ((pTwi->TWI_SR & TWI_SR_RXRDY) == TWI_SR_RXRDY);
}

/**
 * \brief Check if a byte have been sent to TWI.
 * \param pTwi  Pointer to an Twi instance.
 * \return 1 if a byte has been sent so another one can be stored for
 * transmission; otherwise returns 0. This function clears the status register.
 */
uint8_t TWI_ByteSent(Twi *pTwi)
{
    return ((pTwi->TWI_SR & TWI_SR_TXRDY) == TWI_SR_TXRDY);
}

/**
 * \brief Check if current transmission is complete.
 * \param pTwi  Pointer to an Twi instance.
 * \return  1 if the current transmission is complete (the STOP has been sent);
 * otherwise returns 0.
 */
uint8_t TWI_TransferComplete(Twi *pTwi)
{
    return ((pTwi->TWI_SR & TWI_SR_TXCOMP) == TWI_SR_TXCOMP);
}

/**
 * \brief Enables the selected interrupts sources on a TWI peripheral.
 * \param pTwi  Pointer to an Twi instance.
 * \param sources  Bitwise OR of selected interrupt sources.
 */
void TWI_EnableIt(Twi *pTwi, uint32_t sources)
{
    pTwi->TWI_IER = sources;
}

/**
 * \brief Disables the selected interrupts sources on a TWI peripheral.
 * \param pTwi  Pointer to an Twi instance.
 * \param sources  Bitwise OR of selected interrupt sources.
 */
void TWI_DisableIt(Twi *pTwi, uint32_t sources)
{
    pTwi->TWI_IDR = sources;
	
	/* Dummy read */
	//pTwi->TWI_SR;
}

/**
 * \brief Get the current status register of the given TWI peripheral.
 * \note This resets the internal value of the status register, so further
 * read may yield different values.
 * \param pTwi  Pointer to an Twi instance.
 * \return  TWI status register.
 */
uint32_t TWI_GetStatus(Twi *pTwi)
{
    return pTwi->TWI_SR;
}

/**
 * \brief Returns the current status register of the given TWI peripheral, but
 * masking interrupt sources which are not currently enabled.
 * \note This resets the internal value of the status register, so further
 * read may yield different values.
 * \param pTwi  Pointer to an Twi instance.
 */
uint32_t TWI_GetMaskedStatus(Twi *pTwi)
{
    uint32_t status;

    status = pTwi->TWI_SR;
    status &= pTwi->TWI_IMR;

    return status;
}

/**
 * \brief  Sends a STOP condition. STOP Condition is sent just after completing
 *  the current byte transmission in master read mode.
 * \param pTwi  Pointer to an Twi instance.
 */
void TWI_SendSTOPCondition(Twi *pTwi)
{
    pTwi->TWI_CR |= TWI_CR_STOP;
}


/*-------------------------------
Status Register Check Functions
---------------------------------*/
bool TWI_STATUS_SVREAD(uint32_t status){
	return (status & TWI_SR_SVREAD) == TWI_SR_SVREAD;
}

bool TWI_STATUS_SVACC(uint32_t status){
	return (status & TWI_SR_SVACC) == TWI_SR_SVACC;
}

bool TWI_STATUS_GACC(uint32_t status){
	return (status & TWI_SR_GACC) == TWI_SR_GACC;
}

bool TWI_STATUS_EOSACC(uint32_t status){
	return (status & TWI_SR_EOSACC) == TWI_SR_EOSACC;
}

bool TWI_STATUS_NACK(uint32_t status){
	return (status & TWI_SR_NACK) == TWI_SR_NACK;
}


/*---------------------------
Timeout Functions (Blocking)
-----------------------------*/

bool TWI_FailedAcknowledge(Twi *pTwi) {
	return pTwi->TWI_SR & TWI_SR_NACK;
}

bool TWI_WaitTransferComplete(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
		return false;

		if (--_timeout == 0)
		return false;
	}
	return true;
}

bool TWI_WaitByteSent(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_TXRDY) != TWI_SR_TXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
		return false;

		if (--_timeout == 0)
		return false;
	}

	return true;
}

bool TWI_WaitByteReceived(Twi *_twi, uint32_t _timeout) {
	uint32_t _status_reg = 0;
	while ((_status_reg & TWI_SR_RXRDY) != TWI_SR_RXRDY) {
		_status_reg = TWI_GetStatus(_twi);

		if (_status_reg & TWI_SR_NACK)
		return false;

		if (--_timeout == 0)
		return false;
	}

	return true;
} 