/**
Created By: Brandon Braun
Date: 1/2/17
**/

#ifndef _TWI_
#define _TWI_

#include "../libraries.h"

#define TWI_SUCCESS              0
#define TWI_INVALID_ARGUMENT     1
#define TWI_ARBITRATION_LOST     2
#define TWI_NO_CHIP_FOUND        3
#define TWI_RECEIVE_OVERRUN      4
#define TWI_RECEIVE_NACK         5
#define TWI_SEND_OVERRUN         6
#define TWI_SEND_NACK            7
#define TWI_BUSY                 8
#define TWI_ERROR_TIMEOUT        9
#define TWI_READ				 10
#define TWI_WRITE				 11


class TwoWireClass : public RingBufferClass
{
	public:
	TwoWireClass(Twi *pTwi);
	
	uint32_t begin();
	void begin(uint8_t slaveAddress);
	void beginTransmission(uint8_t slaveAddress);
	void endTransmission();
	uint32_t write(uint32_t val);
	uint32_t write(uint32_t *dataBuffer, uint32_t numToWrite);
	uint8_t read();
	uint8_t availableToRead();
	void requestFrom(uint8_t slaveAddress, uint8_t quantity);
	
	//Error and message reporting, todo
	uint32_t TWI_Error();
	uint32_t TWI_Status();
	uint32_t TWI_Message();

	//Allows a user to attach a function to an event
	void onRequest(void(*)(void));
	void onReceive(void(*)(int));
	
	//Interrupt Handler
	void IRQHandler(void);

	enum TwoWireStatus {
		UNINITIALIZED,
		MASTER_IDLE,
		MASTER_SEND,
		MASTER_RECV,
		SLAVE_IDLE,
		SLAVE_RECV,
		SLAVE_SEND
	};
	
	enum TwoWireMessages{
		NO_MESSAGE,
		TRANSMIT_START,
		TRANSMIT_ADDRESS,
		TRANSMIT_DATA,
		TRANSMIT_SUCCESSFUL,
		RECEIVE_START,
		RECEIVE_DATA,
		RECEIVE_SUCCESSFUL
	};
	
	enum TwoWireErrors{
		NO_ERROR,
		ERROR_NACK_ON_ADDRESS_TRANSMIT,
		ERROR_NACK_DURING_TRANSMIT,
		ERROR_ARBLST,
		ERROR_UNKNOWN_INTERRUPT
	};
	
	volatile TwoWireErrors TWIERROR;
	volatile TwoWireMessages TWIMESSAGE;
	volatile TwoWireStatus TWISTATUS;
	
	private:
	
	//Instance specific information
	Twi *twi;
	IRQn_Type IRQnID;
	uint32_t periphID;
	uint32_t SDA, SCL;
	bool initialized = false;
	
	//Read length for interrupt handler
	volatile uint8_t leftToRead = 0;
	
	//TWI clock frequency
	uint32_t twiClock = 400000;

	//Timeouts
	static const uint32_t RECV_TIMEOUT = 1000;
	static const uint32_t XMIT_TIMEOUT = 1000;
	
	/** Initialize Ring Buffers **/
	static const uint32_t I2C_BUFFER_LENGTH = 16;
	
	//Transmit Data (Master Mode)
	uint32_t _txBuffer[I2C_BUFFER_LENGTH];
	RingBufferClass txBuffer = RingBufferClass(_txBuffer, I2C_BUFFER_LENGTH);
	
	//Receive Data (Master Mode)
	uint32_t _rxBuffer[I2C_BUFFER_LENGTH];
	RingBufferClass rxBuffer = RingBufferClass(_rxBuffer, I2C_BUFFER_LENGTH);
	
	//Stored Data (Slave Mode)
	uint32_t _svBuffer[I2C_BUFFER_LENGTH];
	RingBufferClass svBuffer = RingBufferClass(_svBuffer, I2C_BUFFER_LENGTH);
	
	
	struct twi_packet {
		uint8_t iaddr[3];		//TWI address/commands to issue to the other chip (node).
		uint32_t iaddr_length;	//Length of the TWI data address segment (1-3 bytes).
		uint8_t rxLength;		//How many bytes do we want to receive?
		uint8_t chip;			//TWI chip address to communicate with.
		uint8_t registerAccess; //First register accessed in the transmission
	};
	//Stores a single transmission data packet configuration
	twi_packet Packet;
	
	
	void update_MMR(uint32_t rw);
	void update_IADR();
	uint32_t TWI_StartWrite();
	uint32_t TWI_StartRead();
	
	
	/** Attached user functions **/
	void (*onRequestCallback)(void);
	void (*onReceiveCallback)(int);

	void setupHardware();
};

extern uint32_t TWI_ConfigureMaster(Twi *pTwi, uint32_t twck, uint32_t mck);

extern void TWI_SetMaster(Twi *pTwi);

extern uint32_t TWI_SetClock( Twi *pTwi, uint32_t dwTwCk, uint32_t dwMCk );

extern void TWI_ConfigureSlave(Twi *pTwi, uint8_t slaveAddress);

extern void TWI_Start(Twi *pTwi);

extern void TWI_Stop(Twi *pTwi);

extern void TWI_Reset(Twi *pTwi);

extern void TWI_StartRead(Twi *pTwi, uint8_t slaveAddress, uint32_t iaddress, uint8_t isize);

extern uint8_t TWI_ReadByte(Twi *pTwi);

extern void TWI_WriteByte(Twi *pTwi, uint8_t byte);

//extern void TWI_StartWrite(Twi *pTwi, twi_packet *dataPacket);

extern uint32_t TWI_makeAddr(const uint8_t *addr, int len);

extern uint8_t TWI_ByteReceived(Twi *pTwi);

extern uint8_t TWI_ByteSent(Twi *pTwi);

extern uint8_t TWI_TransferComplete(Twi *pTwi);

extern void TWI_EnableIt(Twi *pTwi, uint32_t sources);

extern void TWI_DisableIt(Twi *pTwi, uint32_t sources);

extern uint32_t TWI_GetStatus(Twi *pTwi);

extern uint32_t TWI_GetMaskedStatus(Twi *pTwi);

extern void TWI_SendSTOPCondition(Twi *pTwi);

extern bool TWI_STATUS_SVREAD(uint32_t status);

extern bool TWI_STATUS_SVACC(uint32_t status);

extern bool TWI_STATUS_GACC(uint32_t status);

extern bool TWI_STATUS_EOSACC(uint32_t status);

extern bool TWI_STATUS_NACK(uint32_t status);

extern bool TWI_FailedAcknowledge(Twi *pTwi);

extern bool TWI_WaitTransferComplete(Twi *_twi, uint32_t _timeout);

extern bool TWI_WaitByteSent(Twi *_twi, uint32_t _timeout);

extern bool TWI_WaitByteReceived(Twi *_twi, uint32_t _timeout);

#endif /* #ifndef _TWI_ */