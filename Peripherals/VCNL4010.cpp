
#include "../libraries.h"

VCNL4010::VCNL4010(TwoWireClass *pTwi){
	twi = pTwi;
}

bool VCNL4010::begin(uint8_t addr) {
	slaveAddress = addr;
	
	//Starts up the I2C if it hasn't been initialized already
	twi->begin();

	uint8_t rev = read8(VCNL4010_PRODUCTID);

	if ((rev & 0xF0) != 0x20)
		return false;
	
	//Max LED current (200mA)
	setLEDcurrent(20);
	
	//Set to the lowest frequency
	setFrequency(VCNL4010_390K625);
	
	//Generates an interrupt at proximity data ready
	write8(VCNL4010_INTCONTROL, 0x08);
   return true;
}
 
void VCNL4010::setLEDcurrent(uint8_t c) {
	if (c > 20) c = 20;
	write8(VCNL4010_IRLED, c);
}

void VCNL4010::setFrequency(vcnl4010_led_freq f){
	//Read the current value
	uint8_t r =  read8(VCNL4010_MODTIMING);
	
	//Clear bits 3 and 4 
	r &= 0xE7;
	
	//Write bits 3 and 4 with new values
	r |= f << 3;
	
	//Write the register with new freq setting
	write8(VCNL4010_MODTIMING, r);
}

void VCNL4010::setProxRate(vcnl4010_prox_rate rate){
	
}

uint8_t VCNL4010::getLEDcurrent(void) {
	return read8(VCNL4010_IRLED);
}

uint16_t  VCNL4010::readProximity(void) {
	//Clear the proximity data ready interrupt flag
	uint8_t i = read8(VCNL4010_INTSTAT);
	i &= 0x7F;
	write8(VCNL4010_INTSTAT, i);
	
	//Send command to start proximity measurement
	write8(VCNL4010_COMMAND, VCNL4010_MEASUREPROXIMITY);
	
	uint8_t retries = 0;
	
	while (retries < VCNL4010_TIMEOUT) {
		uint8_t result = read8(VCNL4010_COMMAND);
		
		//If the data is ready, read and return it
		if(result & VCNL4010_PROXIMITYREADY)
			return read16(VCNL4010_PROXIMITYDATA);
		
		++retries;
	}
	
	//If the program gets to this point, timeout occurred
	return false;
}

uint16_t  VCNL4010::readAmbient(void) {
//   uint8_t i = read8(VCNL4010_INTSTAT);
//   i &= ~0x40;
//   write8(VCNL4010_INTSTAT, i);
// 
// 
//   write8(VCNL4010_COMMAND, VCNL4010_MEASUREAMBIENT);
//   while (1) {
//     //Serial.println(read8(VCNL4010_INTSTAT), HEX);
//     uint8_t result = read8(VCNL4010_COMMAND);
//     //Serial.print("Ready = 0x"); Serial.println(result, HEX);
//     if (result & VCNL4010_AMBIENTREADY) {
//       return read16(VCNL4010_AMBIENTDATA);
//     }
//     delay(1);
//   }
	return true;
}


// Read 1 byte from the VCNL4000 at 'address'
uint8_t VCNL4010::read8(uint8_t address){

	twi->beginTransmission(slaveAddress);
	twi->write(address);
	twi->endTransmission();


	twi->requestFrom(slaveAddress, 1);
	while(!twi->availableToRead());
	
	return twi->read() & 0xFF;
}


// Read 2 byte from the VCNL4000 at 'address'
uint16_t VCNL4010::read16(uint8_t address){
	uint16_t data;
	
	//Start the transmission
	twi->beginTransmission(slaveAddress);
	twi->write(address);
	twi->endTransmission();

	twi->requestFrom(slaveAddress, (uint8_t)2);
	while(!twi->availableToRead());
	
	//Read data, shift it over
	data = twi->read();
	data <<= 8;
	
	//Wait for more data to come
	while(!twi->availableToRead());
	
	//Append the new data
	data |= twi->read();

	return data & 0xFFFF;
}

//Write 1 byte
void VCNL4010::write8(uint8_t address, uint8_t data){
	twi->beginTransmission(slaveAddress);
	twi->write(address);
	twi->write(data);
	twi->endTransmission();
}
