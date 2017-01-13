
#include "LSM9DS0.h"

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

/** Hardware SPI Interface **/
LSM9DS0::LSM9DS0(SPIClass* spiInstance, uint32_t spiSettings, uint32_t xmCH, uint32_t xmCS, uint32_t gyroCH, uint32_t gyroCS, int32_t sensorID) {
  _i2c = false;
  
  spi = spiInstance;
  
  //Save hardware pin
  gyroChipSelect = gyroCS;
  xmChipSelect   = xmCS;
  
  //Save SPI Channel information
  gyroSpiChannel = gyroCH;
  xmSpiChannel   = xmCH;
  spiClock		 = 800000L;
  spiConfiguration = spiSettings;
}

bool LSM9DS0::begin(GyroScale gSCL, AccelScale aSCL, MagScale mSCL,
							 GyroDataRate gODR, AccelDataRate aODR, MagDataRate mODR){
	//Setup Comms
	if (_i2c)
		wire->begin();
	else {
		spi->begin();
	
		//Setup the SPI channels for telemetry
		SPI.attachPinToChannel(xmSpiChannel, xmChipSelect);
		SPI.configureChannel(xmSpiChannel, spiConfiguration, spiClock);
		SPI.applyChannelConfig(xmSpiChannel, SPI_HW_CHANNEL_0);
	
		SPI.attachPinToChannel(gyroSpiChannel, gyroChipSelect);
		SPI.configureChannel(gyroSpiChannel, spiConfiguration, spiClock);
		SPI.applyChannelConfig(gyroSpiChannel, SPI_HW_CHANNEL_1);
	}
	
	//Check that the accelerometer is there
	uint32_t id = read8(XMTYPE, REGISTER_WHO_AM_I_XM);
	if((id & 0xFF) != LSM9DS0_XM_ID)
		return false;
		//Serial1.write(id, BYTE);
	
	//Check that the Gyro is there
	id = read8(GYROTYPE, REGISTER_WHO_AM_I_G);
	if((id & 0xFF) != LSM9DS0_G_ID)
		return false;
		//Serial1.write(id, BYTE);
	
	//Store the scales
	gScale = gSCL;
	aScale = aSCL;
	mScale = mSCL;
	
	//Calculate the resolution of each sensor
	calcgRes();
	calcaRes();
	calcmRes();
	
	//Setup Gyro
	initGyro();
	setGyroODR(gODR);
	setGyroScale(gScale);
	
	//Setup Accelerometer
	initAccel();
	setAccelODR(aODR);
	setAccelScale(aScale);
	
	//Setup Magnetometer
	initMag();
	setMagODR(mODR);
	setMagScale(mScale);

	return true;
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/
void LSM9DS0::calibrateLSM9DS0(){
	uint8_t data[6] = {0, 0 ,0 ,0, 0, 0};
	int16_t gyro_bias[3] = {0, 0, 0};
	int16_t accel_bias[3] = {0, 0, 0};
	int samples, ii;
	
	/************************************************************************/
	/* GYRO BIAS                                                            */
	/************************************************************************/
	
	//Enable FIFO mode
	uint8_t c = read8(GYROTYPE, REGISTER_CTRL_REG5_G);
	write8(GYROTYPE, REGISTER_CTRL_REG5_G, c | 0x40);
	
	//Wait
	delayMilliseconds(20);
	
	//Enable FIFO stream mode, watermark at 32 samples
	write8(GYROTYPE, REGISTER_FIFO_CTRL_REG_G, 0x20 | 0x1F);
	
	//Wait for samples to accumulate
	delayMilliseconds(1000);
	
	//Read number of stored samples
	samples = (read8(GYROTYPE, REGISTER_FIFO_SRC_REG_G) & 0x1F);
	
	//Read the stored data
	for(ii=0; ii<samples; ii++){
		readBuffer(GYROTYPE, REGISTER_OUT_X_L_G, 6, data);
		gyro_bias[0] += (((int16_t)data[1] << 8) | data[0]);
		gyro_bias[1] += (((int16_t)data[3] << 8) | data[2]);
		gyro_bias[2] += (((int16_t)data[5] << 8) | data[4]);
	}
	
	//Average data
	gyro_bias[0] /= samples;
	gyro_bias[1] /= samples;
	gyro_bias[2] /= samples;
	
	//Scale to get degrees per second
	float _x = qfp_int2float(gyro_bias[0]);
	float _y = qfp_int2float(gyro_bias[1]);
	float _z = qfp_int2float(gyro_bias[2]);
	
	gBias[0] = qfp_fmul(gRes, _x);
	gBias[1] = qfp_fmul(gRes, _y);
	gBias[2] = qfp_fmul(gRes, _z);
	
	//Disable FIFO
	c = read8(GYROTYPE, REGISTER_CTRL_REG5_G);
	write8(GYROTYPE, REGISTER_CTRL_REG5_G, c & ~0x40);
	
	//Wait
	delayMilliseconds(20);
	
	//Re-enable bypass mode
	write8(GYROTYPE, REGISTER_FIFO_CTRL_REG_G, 0x00);
	
	
	/************************************************************************/
	/* ACCELEROMETER BIAS                                                   */
	/************************************************************************/
	//Enable FIFO
	c = read8(XMTYPE, REGISTER_CTRL_REG0_XM);
	write8(XMTYPE, REGISTER_CTRL_REG0_XM, c | 0x40);
	
	//Wait
	delayMilliseconds(20);
	
	//Enable FIFO stream mode, set watermark at 32 samples
	delayMilliseconds(1000);
	
	//Read number of stored samples
	samples = (read8(XMTYPE, REGISTER_FIFO_SRC_REG));
	
	//Read data stored in the FIFO
	for(ii=0; ii<samples; ii++){
		readBuffer(XMTYPE, REGISTER_OUT_X_L_A, 6, data);
		accel_bias[0] += (((int16_t)data[1] << 8) | data[0]);
		accel_bias[1] += (((int16_t)data[3] << 8) | data[2]);
		
		//Assumes sensor is facing up
		accel_bias[2] += (((int16_t)data[4] << 8) | data[4]) - (int16_t)qfp_float2int(qfp_fdiv(1.0, aRes)); 
	}
	
	//Average the data
	accel_bias[0] /= samples;
	accel_bias[1] /= samples;
	accel_bias[2] /= samples;
	
	//Properly scale to get g's (not m/s2)
	_x = qfp_int2float(accel_bias[0]);
	_y = qfp_int2float(accel_bias[1]);
	_z = qfp_int2float(accel_bias[2]);
	
	aBias[0] = qfp_fmul(aRes, _x);
	aBias[1] = qfp_fmul(aRes, _y);
	aBias[2] = qfp_fmul(aRes, _z);
	
	//Disable FIFO
	c = read8(XMTYPE, REGISTER_CTRL_REG0_XM);
	write8(XMTYPE, REGISTER_CTRL_REG0_XM, c & ~0x40);
	
	//Wait 
	delayMilliseconds(20);
	
	//Re-enable bypass mode
	write8(XMTYPE, REGISTER_FIFO_CTRL_REG, 0x00);
}

void LSM9DS0::read(){
  /* Read all the sensors. */
  readAccel();
  readMag();
  readGyro();
  readTemp();
}

void LSM9DS0::convert(){
	convertAccel();
	convertGyro();
	convertMag();
	convertTemp();
}

void LSM9DS0::readAccel() {
  // Read the accelerometer
  uint8_t temp[6];
  ta = millis();
  readBuffer(XMTYPE, REGISTER_OUT_X_L_A, 6, temp);
	
  //Data comes out with low byte in bits 15-8. Needs to be reversed
  ax = (temp[1] << 8) | temp[0]; // Store x-axis values into ax
  ay = (temp[3] << 8) | temp[2]; // Store y-axis values into ay
  az = (temp[5] << 8) | temp[4]; // Store z-axis values into az
}

void LSM9DS0::readMag() {
  // Read the magnetometer
  uint8_t temp[6];
  tm = millis();
  readBuffer(XMTYPE, REGISTER_OUT_X_L_M, 6, temp);
  
  //Data comes out with low byte in bits 15-8. Needs to be reversed
  mx = (temp[1] << 8) | temp[0]; // Store x-axis values into mx
  my = (temp[3] << 8) | temp[2]; // Store y-axis values into my
  mz = (temp[5] << 8) | temp[4]; // Store z-axis values into mz
}

void LSM9DS0::readGyro() {
  // Read gyro
  uint8_t temp[6];
  tg = millis();
  readBuffer(GYROTYPE, REGISTER_OUT_X_L_G, 6, temp);
  
  //Data comes out with low byte in bits 15-8. Needs to be reversed	
  gx = (temp[1] << 8) | temp[0]; // Store x-axis values into gx
  gy = (temp[3] << 8) | temp[2]; // Store y-axis values into gy
  gz = (temp[5] << 8) | temp[4]; // Store z-axis values into gz
}

void LSM9DS0::readTemp() {
	uint8_t temp[2] = {0,0}; // We'll read two bytes from the temperature sensor into temp
	
	readBuffer(XMTYPE, REGISTER_TEMP_OUT_L_XM, 2, temp); // Read 2 bytes, beginning at OUT_TEMP_L_M
	
	temperature = (((int16_t) temp[1] << 12) | temp[0] << 4 ) >> 4; // Temperature is a 12-bit signed integer
}

void LSM9DS0::convertAccel(){
	
	float _x = calcAccel(ax);
	float _y = calcAccel(ay);
	float _z = calcAccel(az);
	
	//Subtract the calibration bias from the read data to get acceleration 
	//relative to initial orientation
	accelData.x = qfp_fsub(_x, aBias[0]);
	accelData.y = qfp_fsub(_y, aBias[1]);
	accelData.z = qfp_fsub(_z, aBias[2]);
	
	//Do not subtract bias to get absolute acceleration
	accelData.xABS = _x;
	accelData.yABS = _y;
	accelData.zABS = _z;
	
	//Record timestamp
	accelData.time = ta;
}

void LSM9DS0::convertGyro(){
	float _x = calcGyro(gx);
	float _y = calcGyro(ay);
	float _z = calcGyro(az);
	
	//Subtract the calibration bias from the read data to get acceleration
	//relative to initial orientation
	gyroData.x = qfp_fsub(_x, gBias[0]);
	gyroData.y = qfp_fsub(_y, gBias[1]);
	gyroData.z = qfp_fsub(_z, gBias[2]);
	
	//Do not subtract bias to get absolute acceleration
	gyroData.xABS = _x;
	gyroData.yABS = _y;
	gyroData.zABS = _z;
	
	//Record timestamp
	gyroData.time = tg;
}

void LSM9DS0::convertMag(){
	magData.x = calcMag(mx);
	magData.y = calcMag(my);
	magData.z = calcMag(mz);
	magData.time = tm;
}

void LSM9DS0::convertTemp(){
	//To do
}

/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/
void LSM9DS0::write8(bool type, uint16_t reg, uint16_t value){
	uint32_t spiChannel;
	uint16_t data;
	
	//Figure out a few hardware details
	if(type == GYROTYPE){
		//address = LSM9DS0_ADDRESS_GYRO;
		spiChannel = gyroSpiChannel;
	} else {
		//address = LSM9DS0_ADDRESS_ACCELMAG;
		spiChannel = xmSpiChannel;
	}
	
	
	//Write some data
	if(_i2c){
		
	} else {
		//Write to register, telling address to increment on multiple writes
		data = ((reg|0x40) << 8) | value;
		
		spi->write(spiChannel, data, true);
		
		//Dummy read so the internal buffer index is up to date
		spi->read();
    }
}

uint32_t LSM9DS0::read8(bool type, uint16_t reg){
	uint32_t spiChannel;
	uint16_t data;
	
	//Figure out a few hardware details
	if(type == GYROTYPE){
		//address = LSM9DS0_ADDRESS_GYRO;
		spiChannel = gyroSpiChannel;
	} else {
		//address = LSM9DS0_ADDRESS_ACCELMAG;
		spiChannel = xmSpiChannel;
	}
	
	//Read some data
	if(_i2c){
		
		
		return 0u;
	} else {
		//Write address, tell chip to read & increment address on multiple reads
		data = ((reg | 0x80u | 0x40u) << 8) | 0xFFu;
		
		spi->write(spiChannel, data, true);
		
		//Useful data here
		return spi->read() & 0xFFu;
	 }
}

void LSM9DS0::readBuffer(bool type, uint16_t reg, uint32_t len, uint8_t *buffer){
	uint32_t spiChannel;
	uint16_t data;
	
	//Figure out a few hardware details
	if(type == GYROTYPE){
		//address = LSM9DS0_ADDRESS_GYRO;
		spiChannel = gyroSpiChannel;
	} else {
		//address = LSM9DS0_ADDRESS_ACCELMAG;
		spiChannel = xmSpiChannel;
	}
	
	//Read some data
	if(_i2c){
		
	} else {
		//Starting address
		data = ((reg | 0x80) << 8) | 0xFFu;
		
		for(uint32_t x=0; x<len; x++){
			//Access device and clock back data
			spi->write(spiChannel, data, true);
			
			//Store the new data
			buffer[x] = spi->read() & 0xFF;
			
			//Increment the address
			reg = reg + 0x01u;
			data = ((reg | 0x80) << 8) | 0xFFu;
		}
		
		
		
	}
}

/** Calculates the gyroscope resolution based on gScale settings.
Verified working properly **/
void LSM9DS0::calcgRes(){
	switch (gScale){
	case GYROSCALE_245DPS:
		gRes = qfp_fdiv(245.0, 32768.0);
		break;
	
	case GYROSCALE_500DPS:
		gRes = qfp_fdiv(500.0, 32768.0);
		break;
	
	case GYROSCALE_2000DPS:
		gRes = qfp_fdiv(2000.0, 32768.0);
		break;
	}
}

/** Calculates the accelerometer resolution based on aScale settings.
Verified working properly **/
void LSM9DS0::calcaRes(){
	
	if(aScale == ACCELRANGE_16G)
		aRes = qfp_fdiv(16.0, 32768.0);
	else{
		float varA = qfp_fadd(qfp_int2float(aScale), 1.0);
		float varB = qfp_fmul(varA, 2.0);
		aRes = qfp_fdiv(varB, 32768.0);
	}
}

/** Calculates the magnetometer resolution based on mScale settings.
Verified working properly **/
void LSM9DS0::calcmRes(){
	if(mScale == MAGGAIN_2GAUSS)
		mRes = qfp_fdiv(2.0, 32768.0);
	else{
		float varA = qfp_int2float(mScale << 2);
		mRes = qfp_fdiv(varA, 32768.0);
	}
}

void LSM9DS0::initGyro(){
	//Normal mode, enable all axis
	write8(GYROTYPE, REGISTER_CTRL_REG1_G, 0x0F);
	
	//Normal mode, high cutoff frequency
	write8(GYROTYPE, REGISTER_CTRL_REG2_G, 0x00);
	
	//Enable interrupt on Int_G pin (active low), on data ready
	write8(GYROTYPE, REGISTER_CTRL_REG3_G, 0x88);
	
	//Set scale to 245dps
	write8(GYROTYPE, REGISTER_CTRL_REG4_G, 0x00);
	
	//Not much here. Just setting zero.
	write8(GYROTYPE, REGISTER_CTRL_REG5_G, 0x00);
}

void LSM9DS0::setGyroODR(GyroDataRate rate){
	// We need to preserve the other bytes in CTRL_REG1_G. So, first read it:
	uint8_t temp = read8(GYROTYPE, REGISTER_CTRL_REG1_G);
	
	// Then mask out the gyro ODR bits:
	temp &= 0xFF^(0xF << 4);
	
	// Then shift in our new ODR bits:
	temp |= (rate << 4);
	
	// And write the new register value back into CTRL_REG1_G:
	write8(GYROTYPE, REGISTER_CTRL_REG1_G, temp);
}

void LSM9DS0::setGyroScale(GyroScale scale){
	// We need to preserve the other bytes in CTRL_REG4_G. So, first read it:
	uint8_t temp = read8(GYROTYPE, REGISTER_CTRL_REG4_G);
	
	// Then mask out the gyro scale bits:
	temp &= 0xFF^(0x3 << 4);
	
	// Then shift in our new scale bits:
	temp |= scale << 4;
	
	// And write the new register value back into CTRL_REG4_G:
	write8(GYROTYPE, REGISTER_CTRL_REG4_G, temp);
	
	// We've updated the sensor, but we also need to update our class variables
	// First update gScale:
	gScale = scale;
	
	// Then calculate a new gRes, which relies on gScale being set correctly:
	calcgRes();
}

void LSM9DS0::initAccel(){
	//Set zero
	write8(XMTYPE, REGISTER_CTRL_REG0_XM, 0x00);
	
	//100Hz data rate, enable all axis
	write8(XMTYPE, REGISTER_CTRL_REG1_XM, 0x57);
	
	//Scale to 2G
	write8(XMTYPE, REGISTER_CTRL_REG2_XM, 0x00);
	
	//Set interrupt on INT1_XM for data ready
	write8(XMTYPE, REGISTER_CTRL_REG3_XM, 0x04);
}

void LSM9DS0::setAccelODR(AccelDataRate rate){
	// We need to preserve the other bytes in CTRL_REG1_XM. So, first read it:
	uint8_t temp = read8(XMTYPE, REGISTER_CTRL_REG1_XM);
	
	// Then mask out the accel ODR bits:
	temp &= 0xFF^(0xF << 4);
	
	// Then shift in our new ODR bits:
	temp |= (rate << 4);
	
	// And write the new register value back into CTRL_REG1_XM:
	write8(XMTYPE, REGISTER_CTRL_REG1_XM, temp);
}

void LSM9DS0::setAccelScale(AccelScale scale){
	// We need to preserve the other bytes in CTRL_REG2_XM. So, first read it:
	uint8_t temp = read8(XMTYPE, REGISTER_CTRL_REG2_XM);
	
	// Then mask out the accel scale bits:
	temp &= 0xFF^(0x3 << 3);
	
	// Then shift in our new scale bits:
	temp |= scale << 3;
	
	// And write the new register value back into CTRL_REG2_XM:
	write8(XMTYPE, REGISTER_CTRL_REG2_XM, temp);
	
	// We've updated the sensor, but we also need to update our class variables
	// First update aScale:
	aScale = scale;
	
	// Then calculate a new aRes, which relies on aScale being set correctly:
	calcaRes();
}

void LSM9DS0::setAccelBW(AccelBW bandwidth){
	// We need to preserve the other bytes in CTRL_REG2_XM. So, first read it:
	uint8_t temp = read8(XMTYPE, REGISTER_CTRL_REG2_XM);
	
	// Then mask out the accel ABW bits:
	temp &= 0xFF^(0x3 << 7);
	
	// Then shift in our new ODR bits:
	temp |= (bandwidth << 7);
	
	// And write the new register value back into CTRL_REG2_XM:
	write8(XMTYPE, REGISTER_CTRL_REG2_XM, temp);
}

void LSM9DS0::initMag(){
	//100 Hz, enable all axis, enable temp sensor
	write8(XMTYPE, REGISTER_CTRL_REG5_XM, 0x94);
	
	//2 Gauss scale
	write8(XMTYPE, REGISTER_CTRL_REG6_XM, 0x00);
	
	//Continuous conversion mode
	write8(XMTYPE, REGISTER_CTRL_REG7_XM, 0x00);
	
	//Mag data ready interrupt on INT2_XM
	write8(XMTYPE, REGISTER_CTRL_REG4_XM, 0x04);
	
	//Enable interrupts for mag, active low, push pull
	write8(XMTYPE, REGISTER_INT_CTRL_REG_M, 0x09);
}

void LSM9DS0::setMagODR(MagDataRate rate){
	// We need to preserve the other bytes in CTRL_REG5_XM. So, first read it:
	uint8_t temp = read8(XMTYPE, REGISTER_CTRL_REG5_XM);
	
	// Then mask out the mag ODR bits:
	temp &= 0xFF^(0x7 << 2);
	
	// Then shift in our new ODR bits:
	temp |= (rate << 2);
	
	// And write the new register value back into CTRL_REG5_XM:
	write8(XMTYPE, REGISTER_CTRL_REG5_XM, temp);
}

void LSM9DS0::setMagScale(MagScale scale){
	// We need to preserve the other bytes in CTRL_REG6_XM. So, first read it:
	uint8_t temp = read8(XMTYPE, REGISTER_CTRL_REG6_XM);
	
	// Then mask out the mag scale bits:
	temp &= 0xFF^(0x3 << 5);
	
	// Then shift in our new scale bits:
	temp |= scale << 5;
	
	// And write the new register value back into CTRL_REG6_XM:
	write8(XMTYPE, REGISTER_CTRL_REG6_XM, temp);
	
	// We've updated the sensor, but we also need to update our class variables
	// First update mScale:
	mScale = scale;
	
	// Then calculate a new mRes, which relies on mScale being set correctly:
	calcmRes();
}

float LSM9DS0::calcGyro(int16_t gyro){
	return qfp_fmul(gRes, qfp_int2float(gyro));
}

float LSM9DS0::calcAccel(int16_t accel){
	return qfp_fmul(aRes, qfp_int2float(accel));
}

float LSM9DS0::calcMag(int16_t mag){
	return qfp_fmul(mRes, qfp_int2float(mag));
}