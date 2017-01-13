
/**
 * \file
 *
 * Interface for Serial Peripheral Interface (SPI) controller.
 *
 * Currently needs support added for Interrupts, expanded chip
 * select configurations, and variable frequency settings that
 * are not limited to the CLOCK_DIV options.
 */

#ifndef _SPI_
#define _SPI_

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include "../libraries.h"


#define MAX_SW_CHANNELS 12
#define MAX_HW_CHANNEL 4

/************************************************************************/
/*MACROS                                                                */
/************************************************************************/

/** Calculate the Peripheral Chip Select (PCS) field value given the chip select NPCS value
	PG. 695 in Datasheet */
#define SPI_PCS(npcs)       ((~(1 << (npcs)) & 0xF) << 16)

/** Calculates the value of the Chip Select Register (CSR) Serial Clock Baud Rate (SCBR) field given the baudrate and MCK. */
#define SPI_SCBR(baudrate, MASTER_CLOCK) ((uint32_t) ((MASTER_CLOCK) / (baudrate)) << 8)

/** Calculates the value of the Chip Select Register (CSR) Delay Before Spi Clock (DLYBS) field given the desired delay (in ns) */
#define SPI_DLYBS(delay, MASTER_CLOCK) ((uint32_t) ((((MASTER_CLOCK) / 1000000) * (delay)) / 1000) << 16)

/** Calculates the value of the Chip Select Register (CSR) Delay Between Consecutive Transfers (DLYBCT) field given the desired delay (in ns) */
#define SPI_DLYBCT(delay, MASTER_CLOCK) ((uint32_t) ((((MASTER_CLOCK) / 1000000) * (delay)) / 32000) << 24)


/************************************************************************/
/* General Definitions                                                  */
/************************************************************************/

/** SPI Clock Frequencies*/
#define SPI_CLOCK_FREQ(baud)	((uint32_t)(MASTER_CLOCK/baud)<<8) //Variable Frequency, baud in Hz


/*------------------------------------------------------------------------------ */

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/


extern void SPI_Enable( Spi* spi ) ;
extern void SPI_Disable( Spi* spi ) ;
extern void SPI_EnableIt( Spi* spi, uint32_t dwSources ) ;
extern void SPI_DisableIt( Spi* spi, uint32_t dwSources ) ;

extern void SPI_Configure( Spi* spi, uint32_t id, uint32_t configuration ) ;
extern void SPI_ConfigureNPCS( Spi* spi, uint32_t npcs, uint32_t configuration ) ;

extern uint32_t SPI_Read( Spi* spi ) ;
extern void SPI_Write( Spi* spi, uint32_t npcs, uint16_t data ) ;

extern uint32_t SPI_GetStatus( Spi* spi ) ;
extern uint32_t SPI_IsFinished( Spi* pSpi ) ;


class SPIClass : public RingBufferClass
{
public:
	/** General Interface **/
	void		begin();
	void		write(uint32_t channel, uint16_t data, bool CSNAAT = true);
	uint32_t	read();
	void		end();
	void attachPinToChannel(uint32_t channel, uint32_t pin);
	void configureChannel(uint32_t channel, uint32_t config = SPI_CSR_BITS_8_BIT, uint32_t freq = 4000000);
	void applyChannelConfig(uint32_t channel, SPIHW hardwareChannel = RANDOM);
	
	
	/** Interrupt Handler **/
	void IRQHandler();
	
	/** Constructor **/
	SPIClass();
	
	
private:
	Spi *spi;						//The one and only SPI instance for this board
	IRQn_Type IRQnID;				//Interrupt request ID
	uint32_t periphID;				//Peripheral ID
	bool initialized;				//Is this instance initialized?
	volatile uint32_t hardwareMode; //Keeps track of MASTER or SLAVE mode
	
	/** Ring Buffer Instantiation **/
	uint32_t _rxBuffer[16];
	RingBufferClass rxBuffer = RingBufferClass(_rxBuffer, 16);
	
	/** Settings for each channel **/
	struct csConfig{
		//Register settings
		uint32_t CPOL;
		uint32_t NCPHA;
		uint32_t BITS;
		uint32_t FREQ;

/* Not used because of software control of CS pin
		uint32_t CSAAT;
		uint32_t DLYBS;
		uint32_t DLYBCT;
*/
		//Attached chip select information
		uint32_t DIGPIN;
		uint32_t MASK;
		Pio *PORT;
		
		//Hardware information
		bool attached = false;
		uint32_t attachedChannel;
	};
	csConfig settings[MAX_SW_CHANNELS];
	uint32_t lastChannelNum;

	/** Hardware intitializer **/
	void init_SPI_MASTER();
};

#endif 

