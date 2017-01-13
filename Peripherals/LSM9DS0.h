/***************************************************************************
  This is a library for the LSM9DS0 Accelerometer and magnentometer/compass

  Designed specifically to work with the Adafruit LSM9DS0 Breakouts

  These sensors use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#ifndef __LSM9DS0_H__
#define __LSM9DS0_H__


#include "../libraries.h"

#define LSM9DS0_ADDRESS_ACCELMAG			(0x1D)       
#define LSM9DS0_ADDRESS_GYRO				(0x6B)     
#define LSM9DS0_XM_ID						(0x49u)
#define LSM9DS0_G_ID						(0xD4)

//Linear Acceleration: mg per LSB
#define LSM9DS0_ACCEL_MG_LSB_2G				(0.061F)
#define LSM9DS0_ACCEL_MG_LSB_4G				(0.122F)
#define LSM9DS0_ACCEL_MG_LSB_6G				(0.183F)
#define LSM9DS0_ACCEL_MG_LSB_8G				(0.244F)
#define LSM9DS0_ACCEL_MG_LSB_16G			(0.732F) // Is this right? Was expecting 0.488F

//Magnetic Field Strength: gauss range
#define LSM9DS0_MAG_MGAUSS_2GAUSS			(0.08F)
#define LSM9DS0_MAG_MGAUSS_4GAUSS			(0.16F)
#define LSM9DS0_MAG_MGAUSS_8GAUSS			(0.32F)
#define LSM9DS0_MAG_MGAUSS_12GAUSS			(0.48F)

//Angular Rate: dps per LSB
#define LSM9DS0_GYRO_DPS_DIGIT_245DPS		(0.00875F)
#define LSM9DS0_GYRO_DPS_DIGIT_500DPS		(0.01750F)
#define LSM9DS0_GYRO_DPS_DIGIT_2000DPS		(0.07000F)

//Temperature: LSB per degree Celsius
#define LSM9DS0_TEMP_LSB_DEGREE_CELSIUS		(8)  // 1°C = 8, 25° = 200, etc.

#define GYROTYPE							(true)
#define XMTYPE								(false)


typedef enum {
	REGISTER_WHO_AM_I_G          = 0x0F,
	REGISTER_CTRL_REG1_G         = 0x20,
	REGISTER_CTRL_REG2_G		 = 0x21,
	REGISTER_CTRL_REG3_G         = 0x22,
	REGISTER_CTRL_REG4_G         = 0x23,
	REGISTER_CTRL_REG5_G		 = 0x24,
	REGISTER_OUT_X_L_G           = 0x28,
	REGISTER_OUT_X_H_G           = 0x29,
	REGISTER_OUT_Y_L_G           = 0x2A,
	REGISTER_OUT_Y_H_G           = 0x2B,
	REGISTER_OUT_Z_L_G           = 0x2C,
	REGISTER_OUT_Z_H_G           = 0x2D,
	REGISTER_FIFO_CTRL_REG_G	 = 0x28,
	REGISTER_FIFO_SRC_REG_G		 = 0x2F
} GyroRegisters;
    
typedef enum {
	REGISTER_TEMP_OUT_L_XM       = 0x05,
	REGISTER_TEMP_OUT_H_XM       = 0x06,
	REGISTER_STATUS_REG_M        = 0x07,
	REGISTER_OUT_X_L_M           = 0x08,
	REGISTER_OUT_X_H_M           = 0x09,
	REGISTER_OUT_Y_L_M           = 0x0A,
	REGISTER_OUT_Y_H_M           = 0x0B,
	REGISTER_OUT_Z_L_M           = 0x0C,
	REGISTER_OUT_Z_H_M           = 0x0D,
	REGISTER_WHO_AM_I_XM         = 0x0F,
	REGISTER_INT_CTRL_REG_M      = 0x12,
	REGISTER_INT_SRC_REG_M       = 0x13,
	REGISTER_CTRL_REG0_XM		 = 0x1F,
	REGISTER_CTRL_REG1_XM        = 0x20,
	REGISTER_CTRL_REG2_XM        = 0x21,
	REGISTER_CTRL_REG3_XM        = 0x22,
	REGISTER_CTRL_REG4_XM		 = 0x23,
	REGISTER_CTRL_REG5_XM        = 0x24,
	REGISTER_CTRL_REG6_XM        = 0x25,
	REGISTER_CTRL_REG7_XM        = 0x26,
	REGISTER_OUT_X_L_A           = 0x28,
	REGISTER_OUT_X_H_A           = 0x29,
	REGISTER_OUT_Y_L_A           = 0x2A,
	REGISTER_OUT_Y_H_A           = 0x2B,
	REGISTER_OUT_Z_L_A           = 0x2C,
	REGISTER_OUT_Z_H_A           = 0x2D,
	REGISTER_FIFO_CTRL_REG		 = 0x2E,
	REGISTER_FIFO_SRC_REG	     = 0x2F
} MagAccelRegisters;
    
typedef enum {
	GYROSCALE_245DPS,  // +/- 245 degrees per second rotation
	GYROSCALE_500DPS,  // +/- 500 degrees per second rotation
	GYROSCALE_2000DPS  // +/- 2000 degrees per second rotation
} GyroScale;
    
typedef enum {
	ACCELRANGE_2G,
	ACCELRANGE_4G,
	ACCELRANGE_6G,
	ACCELRANGE_8G,
	ACCELRANGE_16G
} AccelScale;
    
typedef enum {
	MAGGAIN_2GAUSS,  // +/- 2 gauss
	MAGGAIN_4GAUSS,  // +/- 4 gauss
	MAGGAIN_8GAUSS,  // +/- 8 gauss
	MAGGAIN_12GAUSS  // +/- 12 gauss
} MagScale;
    
typedef enum {
	//                         ODR (Hz) --- Cutoff
	GYRO_ODR_95_BW_125  = 0x0, //   95         12.5
	GYRO_ODR_95_BW_25   = 0x1, //   95          25
	// 0x2 and 0x3 define the same data rate and bandwidth
	GYRO_ODR_190_BW_125 = 0x4, //   190        12.5
	GYRO_ODR_190_BW_25  = 0x5, //   190         25
	GYRO_ODR_190_BW_50  = 0x6, //   190         50
	GYRO_ODR_190_BW_70  = 0x7, //   190         70
	GYRO_ODR_380_BW_20  = 0x8, //   380         20
	GYRO_ODR_380_BW_25  = 0x9, //   380         25
	GYRO_ODR_380_BW_50  = 0xA, //   380         50
	GYRO_ODR_380_BW_100 = 0xB, //   380         100
	GYRO_ODR_760_BW_30  = 0xC, //   760         30
	GYRO_ODR_760_BW_35  = 0xD, //   760         35
	GYRO_ODR_760_BW_50  = 0xE, //   760         50
	GYRO_ODR_760_BW_100 = 0xF, //   760         100
} GyroDataRate;
    
typedef enum {
	ACCELDATARATE_POWERDOWN,
	ACCELDATARATE_3_125HZ,
	ACCELDATARATE_6_25HZ,
	ACCELDATARATE_12_5HZ,
	ACCELDATARATE_25HZ,
	ACCELDATARATE_50HZ,
	ACCELDATARATE_100HZ,
	ACCELDATARATE_200HZ,
	ACCELDATARATE_400HZ,
	ACCELDATARATE_800HZ,
	ACCELDATARATE_1600HZ
} AccelDataRate;
    
typedef enum{
	ACCEL_ABW_773,
	ACCEL_ABW_194,
	ACCEL_ABW_362,
	ACCEL_ABW_50
} AccelBW;
    
typedef enum {
	MAGDATARATE_3_125HZ,
	MAGDATARATE_6_25HZ,
	MAGDATARATE_12_5HZ,
	MAGDATARATE_25HZ,
	MAGDATARATE_50HZ,
	MAGDATARATE_100HZ
} MagDataRate;


class LSM9DS0
{
	
public:
    LSM9DS0(SPIClass* spiInstance, uint32_t spiSettings,  uint32_t xmCH, uint32_t xmCS, uint32_t gyroCH, uint32_t gyroCS, int32_t sensorID = 0);

    //Stores the signed output data in 8+24 bit fixed notation
    typedef struct vector_s {
      float x, xABS;
      float y, yABS;
      float z, zABS;
	  uint32_t time;
    } Vector;
    
    Vector  accelData;    // Last read accelerometer data will be available here
    Vector  magData;      // Last read magnetometer data will be available here
    Vector  gyroData;     // Last read gyroscope data will be available here
    int16_t temperature;  // Last read temperature data will be available here
    
    bool     begin       (GyroScale gSCL, AccelScale aSCL, MagScale mSCL, 
						  GyroDataRate gODR, AccelDataRate aODR, MagDataRate mODR);
    
	void calibrateLSM9DS0();
	
	//Read raw and convert for all sensors at once
	void read();
	void convert();
	
	//Read raw data
    void readAccel();
    void readMag();
    void readGyro();
    void readTemp();
	
	//Calculate into useful data
	void convertAccel();
	void convertGyro();
	void convertMag();
	void convertTemp();
	
	//Configuration
	void setGyroODR(GyroDataRate rate);
	void setGyroScale(GyroScale scale);
	void setAccelODR(AccelDataRate rate);
	void setAccelScale(AccelScale scale);
	void setAccelBW(AccelBW bandwidth);
	void setMagODR(MagDataRate rate);
	void setMagScale(MagScale scale);
	
	
    void write8      (bool type, uint16_t reg, uint16_t value );
	void readBuffer  (bool type, uint16_t reg, uint32_t len, uint8_t *buffer);
    uint32_t read8   (bool type, uint16_t reg);
	
	
	//Stores the raw signed data from the LSM9DS0
	int16_t gx, gy, gz;
	int16_t ax, ay, az;
	int16_t mx, my, mz;
	
	//Stores time stamp associated with above raw data
	uint32_t tg, ta, tm;
	
private:
    bool _i2c;
	SPIClass* spi;
    TwoWireClass* wire;
	
	//Hardware settings
	uint32_t spiConfiguration, spiClock;
	uint32_t gyroSpiChannel, xmSpiChannel;
    uint32_t gyroChipSelect, xmChipSelect;
	
	//Store the current scale range for each sensor
	GyroScale  gScale;
	AccelScale aScale;
	MagScale   mScale;
	
	//Used for sensor calibration on startup
	float aBias[3] = {0.0, 0.0, 0.0};
	float gBias[3] = {0.0, 0.0, 0.0}; 
	
	//Store calculated sensor resolutions from calcgRes, calcaRes, and calcmRes
	float gRes; // DPS  /ADC Tick
	float aRes; // Gauss/ADC tick
	float mRes; // g's  /ADC tick
	
	//Finds the resolution of each sensor
	void calcgRes(); //Calculate DPS/ADC tick, stored in gRes variable
	void calcaRes(); //Calculate Gs/ADC tick, stored in mRes variable
	void calcmRes(); //Calculate g/ADC tick, stored in aRes variable
	
	//Converts raw int16_t data from LSM9DS0 into usable values
	float calcGyro(int16_t gyro);
	float calcAccel(int16_t accel);
	float calcMag(int16_t mag);
	
	//Setup functions
	void initGyro();
	void initAccel();
	void initMag();
};

#endif
