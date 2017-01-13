/*
 * definitions.h
 *
 * Created: 12/9/2016 7:39:22 PM
 *  Author: codex
 */ 


#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define NONE (0u)
#define MASTER_CLOCK 84000000L

#define MASTER 1u
#define SLAVE 0u

typedef enum _InterruptPriority{
	PRIOR_SERIAL = 0u, //Highest priority
	PRIOR_SPI = 1u,
	PRIOR_I2C = 2u,
	PRIOR_TIMER = 3u
	
} InterruptPriority;

typedef enum _SIPrefix{
	yotta,
	zetta,
	exa,
	peta,
	tera,
	giga,
	mega,
	kilo,
	milli,
	micro,
	nano,
	pico,
	femto,
	atto,
	zepto,
	yocto
}SIPrefix;

typedef enum _tm{
	NanoSeconds,
	MicroSeconds,
	MilliSeconds,
	Seconds,
	Minutes,
	Hours
} Time;

typedef enum _bit{
	BYTE,
	HALFWORD,
	WORD,
	_8BIT,
	_16BIT,
	_24BIT,
	_32BIT
} BitLength;

typedef enum _BITORD{
	MSBFIRST = 0x1u,
	LSBFIRST = 0x0u
} BITORD;

/************************************************************************/
/* SPI DEFINITIONS                                                      */
/************************************************************************/
typedef enum _SPIBIT{
	SPI_8BIT = 0x00u,
	SPI_9BIT = 0x10u,
	SPI_10BIT = 0x20u,
	SPI_11BIT =	0x30u,
	SPI_12BIT =	0x40u,
	SPI_13BIT =	0x50u,
	SPI_14BIT =	0x60u,
	SPI_15BIT =	0x70u,
	SPI_16BIT =	0x80u
} SPIBIT;

typedef enum _SPIHW{
	SPI_HW_CHANNEL_0 = 0x0u,
	SPI_HW_CHANNEL_1 = 0x1u,
	SPI_HW_CHANNEL_2 = 0x2u,
	SPI_HW_CHANNEL_3 = 0x3u,
	RANDOM
} SPIHW;

/************************************************************************/
/* TIMER DEFINITIONS                                                    */
/************************************************************************/


typedef enum catpureType{
	DISABLED,
	RISING,
	FALLING,
	EDGE
} CaptureType;

#endif /* DEFINITIONS_H_ */