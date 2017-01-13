#ifndef _VCNL4010_H_
#define _VCNL4010_H_

#include "../libraries.h"

// the i2c address
#define VCNL4010_I2CADDR_DEFAULT 0x13

// commands and constants
#define VCNL4010_COMMAND 0x80
#define VCNL4010_PRODUCTID 0x81
#define VCNL4010_PROXRATE 0x82
#define VCNL4010_IRLED 0x83
#define VCNL4010_AMBIENTPARAMETER 0x84
#define VCNL4010_AMBIENTDATA 0x85
#define VCNL4010_PROXIMITYDATA 0x87
#define VCNL4010_INTCONTROL 0x89
#define VCNL4010_PROXIMITYADJUST 0x8A
#define VCNL4010_INTSTAT 0x8E
#define VCNL4010_MODTIMING 0x8F
#define VCNL4010_SELFTIMED 0x01
#define VCNL4010_MEASUREAMBIENT 0x10
#define VCNL4010_MEASUREPROXIMITY 0x08
#define VCNL4010_AMBIENTREADY 0x40
#define VCNL4010_PROXIMITYREADY 0x20
#define VCNL4010_TIMEOUT 5

typedef enum{
	VCNL4010_3M125   = 3, //3.125 MHz
	VCNL4010_1M5625  = 2, //1.5625 MHz
	VCNL4010_781K25  = 1, //781.25 kHz
	VCNL4010_390K625 = 0, //390.625 kHz
} vcnl4010_led_freq;

typedef enum{
	VCNL4010_1_95,
	VCNL4010_3_90625,
	VCNL4010_7_8125,
	VCNL4010_16_625,
	VCNL4010_31_25,
	VCNL4010_62_5,
	VCNL4010_125,
	VCNL4010_250
} vcnl4010_prox_rate;


  
class VCNL4010 {
public:
	VCNL4010(TwoWireClass *pTwi);
	bool begin(uint8_t a = VCNL4010_I2CADDR_DEFAULT);

	
	void setLEDcurrent(uint8_t c);
	void setFrequency(vcnl4010_led_freq f);
	void setProxRate(vcnl4010_prox_rate rate);
	
	uint8_t getLEDcurrent(void);
	uint16_t readProximity(void);
	uint16_t readAmbient(void);

private:

	void write8(uint8_t address, uint8_t data);
	uint16_t read16(uint8_t address);
	uint8_t read8(uint8_t address);
	
	uint8_t slaveAddress;
	TwoWireClass *twi;
};

#endif //_VCNL4010_H_