#ifndef _LIBRARIES_
#define _LIBRARIES_

/*
 * Core and peripherals registers definitions
 */
#include "sam.h"
#include "include/variant.h"

//Dependencies
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/************************************************************************/
/* Temporary                                                            */
/************************************************************************/



/************************************************************************/
/* Global Definitions                                                   */
/************************************************************************/
#include "include/definitions.h"

/************************************************************************/
/* Most of these libraries have been customized. Use with caution.      */
/************************************************************************/
#include "include/dacc.h"
#include "include/ioController.h"
#include "include/pio.h"
#include "include/pmc.h"
#include "include/pwmc.h"
#include "include/ringbuffer.h"
#include "include/rng.h"
#include "include/rtc.h"
#include "include/rtt.h"
#include "include/scheduler.h"
#include "include/spi.h"
#include "include/timer.h"
#include "include/tc.h"
#include "include/timetick.h"
#include "include/twi.h"
#include "include/UART.h"
#include "include/USART.h"


/************************************************************************/
/* Peripheral Libraries                                                 */
/************************************************************************/
#include "Peripherals/Sensor.h"
#include "Peripherals/LSM9DS0.h"
#include "Peripherals/VCNL4010.h"


/************************************************************************/
/* Tertiary Libraries                                                   */
/************************************************************************/
#include "../Tertiary/Qfplib/qfplib-m3.h"
#include "../Tertiary/Qfplib/qfplib.h"
#include "../Tertiary/Qfplib/qfpio.h"

/************************************************************************/
/* Global Class Objects                                                 */
/************************************************************************/
/*
Task Scheduler
*/
extern Scheduler taskList;

/*
I2C
*/
extern TwoWireClass Wire;
extern TwoWireClass Wire1;

/*
Serial
*/
extern UARTClass  Serial;
extern USARTClass Serial1;
extern USARTClass Serial2;
extern USARTClass Serial3;

/*
SPI
*/
extern SPIClass SPI;


/*
TIMER
*/
extern SystemTickClass milliSysTick;



#endif /* _LIBRARIES_ */
