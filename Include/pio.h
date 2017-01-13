/*
 * pio.h
 *
 * Created: 11/23/2015 8:18:36 PM
 *  Author: Brandon
 */ 


#ifndef PIO_H_
#define PIO_H_

#include "../libraries.h"

/********General Constants********/
#define LOW 0x0
#define HIGH 0x1
#define OUTPUT 0x0
#define INPUT 0x1
#define PERIPHERAL 0x20
#define INPUT_PULLUP 0x2
#define OUTPUT_DATA 0x3
#define PIN_DATA 0x4
#define FALSE 0x0
#define TRUE 0x1

/*  Default pin configuration (no attribute). */
#define PIO_DEFAULT                 (0u << 0)
/*  The internal pin pull-up is active. */
#define PIO_PULLUP                  (1u << 0)
/*  The internal glitch filter is active. */
#define PIO_DEGLITCH                (1u << 1)
/*  The pin is open-drain. */
#define PIO_OPENDRAIN               (1u << 2)
/*  The internal de-bouncing filter is active. */
#define PIO_DEBOUNCE                (1u << 3)
/*  Enable additional interrupt modes. */
#define PIO_IT_AIME                 (1u << 4)
/*  Interrupt High Level/Rising Edge detection is active. */
#define PIO_IT_RE_OR_HL             (1u << 5)
/*  Interrupt Edge detection is active. */
#define PIO_IT_EDGE                 (1u << 6)
/*  Low level interrupt is active */
#define PIO_IT_LOW_LEVEL            (0               | 0 | PIO_IT_AIME)
/*  High level interrupt is active */
#define PIO_IT_HIGH_LEVEL           (PIO_IT_RE_OR_HL | 0 | PIO_IT_AIME)
/*  Falling edge interrupt is active */
#define PIO_IT_FALL_EDGE            (0               | PIO_IT_EDGE | PIO_IT_AIME)
/*  Rising edge interrupt is active */
#define PIO_IT_RISE_EDGE            (PIO_IT_RE_OR_HL | PIO_IT_EDGE | PIO_IT_AIME)


#define PIN_ATTR_COMBO				(1u<<0)
#define PIN_ATTR_ANALOG				(1u<<1)
#define PIN_ATTR_DIGITAL			(1u<<2)
#define PIN_ATTR_PWM				(1u<<3)
#define PIN_ATTR_TIMER				(1u<<4)


/********Board Pin Names********/
static const uint32_t RX0 = 0;
static const uint32_t TX0 = 1;
static const uint32_t RX1 = 19;
static const uint32_t TX1 = 18;
static const uint32_t RX2 = 17;
static const uint32_t TX2 = 16;
static const uint32_t RX3 = 15;
static const uint32_t TX3 = 14;
static const uint32_t A0 = 54;
static const uint32_t A1 = 55;
static const uint32_t A2 = 56;
static const uint32_t A3 = 57;
static const uint32_t A4 = 58;
static const uint32_t A5 = 59;
static const uint32_t A6 = 60;
static const uint32_t A7 = 61;
static const uint32_t A8 = 62;
static const uint32_t A9 = 63;
static const uint32_t A10 = 64;
static const uint32_t A11 = 65;
static const uint32_t CANRX = 68;
static const uint32_t CANTX = 69;
static const uint32_t SDA1 = 70;
static const uint32_t SCL1 = 71;
static const uint32_t SS0  = 10;
static const uint32_t SS1  = 4;
static const uint32_t SS2  = 52;
static const uint32_t SS3  = 78;
static const uint32_t MOSI = 75;
static const uint32_t MISO = 74;
static const uint32_t SCK  = 76;

typedef enum _pioType{
	PIO_NOT_A_PIN,
	PIO_PERIPH_A,
	PIO_PERIPH_B,
	PIO_INPUT,
	PIO_OUTPUT_0,
	PIO_OUTPUT_1
} EpioType;

/* Definitions and types for pins */
typedef enum _EAnalogChannel
{
	NO_ADC=-1,
	ADC0=0,
	ADC1,
	ADC2,
	ADC3,
	ADC4,
	ADC5,
	ADC6,
	ADC7,
	ADC8,
	ADC9,
	ADC10,
	ADC11,
	ADC12,
	ADC13,
	ADC14,
	ADC15,
	DA0,
	DA1
} EAnalogChannel ;

#define ADC_CHANNEL_NUMBER_NONE 0xFFFFFFFF

// Definitions for PWM channels
typedef enum _EPWMChannel
{
	NOT_ON_PWM=-1,
	PWM_CH0=0,
	PWM_CH1,
	PWM_CH2,
	PWM_CH3,
	PWM_CH4,
	PWM_CH5,
	PWM_CH6,
	PWM_CH7
} EPWMChannel ;

// Definitions for TC channels
typedef enum _ETCChannel
{
	NOT_ON_TIMER=-1,
	TC0_CHA0=0,
	TC0_CHB0,
	TC0_CHA1,
	TC0_CHB1,
	TC0_CHA2,
	TC0_CHB2,
	TC1_CHA3,
	TC1_CHB3,
	TC1_CHA4,
	TC1_CHB4,
	TC1_CHA5,
	TC1_CHB5,
	TC2_CHA6,
	TC2_CHB6,
	TC2_CHA7,
	TC2_CHB7,
	TC2_CHA8,
	TC2_CHB8
} ETCChannel ;


/********GPIO Pin Description********/
typedef struct _PinDescription
{
	Pio* port;
	uint32_t pinMask;
	uint32_t peripheralID;
	EpioType peripheralType;
	uint32_t pinConfiguration;
	uint32_t pinAttribute;
	EAnalogChannel analogChannel;
	EAnalogChannel adcChannelNumber;
	EPWMChannel pwmChannel;
	ETCChannel tccChannel;
} PinDescription;

extern const PinDescription pinCharacteristic[];


extern void PIO_enablePin(Pio* port, const uint32_t pinMask);
extern void PIO_disablePin(Pio* port, const uint32_t pinMask);
extern void PIO_setOutput(Pio* port, const uint32_t pinMask, const uint32_t defaultValue,
const uint32_t multiDriveEN, const uint32_t pullUpEN);
extern void PIO_setInput(Pio* port, const uint32_t pinMask, const uint32_t attribute);
extern void PIO_setPin(Pio* port, const uint32_t pinMask);
extern void PIO_clearPin(Pio* port, const uint32_t pinMask);
extern void PIO_pullUpEnable(Pio* port, const uint32_t pinMask);
extern void PIO_pullUpDisable(Pio* port, const uint32_t pinMask);
extern void PIO_disableInterrupt(Pio* port, const uint32_t pinMask);
extern void PIO_enableInterrupt(Pio* port, const uint32_t pinMask);
extern void PIO_setPeripheral(Pio* port, const uint32_t pinMask, const EpioType type);
extern uint32_t PIO_configurePin(Pio* port, const uint32_t pinMask, const EpioType type, const uint32_t attribute, const uint32_t ioMode);
extern uint32_t PIO_readOutputDataStatus(Pio* port, const uint32_t pinMask);
extern uint32_t PIO_readPinDataStatus(Pio* port, const uint32_t pinMask);




#endif /* PIO_H_ */