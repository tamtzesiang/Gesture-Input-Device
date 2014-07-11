/*
=================================================================================
 Name        : pin.h
 Author      : Le Dang Dung - LeeDangDung@gmail.com
 Version     : 0.1
 Description : Simple Port/Pin operations, defined to be compatible with
 Arduino libraries:
 - pairs of Port.Pin (e.g. "p0.0") are mapped to single Digital Pins (i.e "0")
 - this version supports
   + digitalWrite
   + digitalRead
   + pinMode
   + attachInterrupt/detachInterrupt
   + shiftout
   + _delay_ms

 Tested board : LPC1769

================================================================================
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
================================================================================
 */
#include "LPC17xx.h"

#ifndef PIN_H_
#define PIN_H_

// keywords
#define LSBFIRST  0
#define MSBFIRST  1
#define LOW       0
#define HIGH      1
#define INPUT     0
#define OUTPUT    1
#define true      1
#define false     0

#define EXTERNAL_NUM_INTERRUPTS 5
#define EXTERNAL_INT_0 0
#define EXTERNAL_INT_1 1
#define EXTERNAL_INT_2 2
#define EXTERNAL_INT_3 3
#define EXTERNAL_INT_4 4



typedef void (*voidFuncPtr)(void);
typedef unsigned char bool;

// bit set
#define _BV(bit) (0x1 << (bit))

// LPC specific porting parameters:
// ***************************** GPIO *********************************** //
#define PINCON    LPC_PINCON   // select pin function

// pin operation registers
#define REG_SET   FIOSET
#define REG_CLR   FIOCLR
#define REG_DIR   FIODIR
#define REG_DATA  FIOPIN

#define PINFUNC_GPIO    0
#define PINFUNC_SECOND  1   // look up user manual for function :)
#define PINFUNC_THIRD   2   // look up user manual for function :)
#define PINFUNC_FOURTH  3   // look up user manual for function :)

typedef LPC_GPIO_TypeDef gpio_type;

#define GPIO0     0
#define GPIO1     1
#define GPIO2     2
#define GPIO3     3
#define GPIO4     4

// macros for our program, PORT UNRESOLVED (NEED TO PROVIDE PORT#), fast...
// proclaim pin as GPIO, this one is very LPC specific, redefine it if necessary on the target platform
#define GPIOPORT0SEL(PIN)              ((PIN) < 16) ? ( PINCON->PINSEL0 &= ~(0x3<<((PIN)*2)) ) : ( PINCON->PINSEL1 &= ~(0x3<<((PIN-16)*2)) )
#define GPIOPORT1SEL(PIN)              ((PIN) < 16) ? ( PINCON->PINSEL2 &= ~(0x3<<((PIN)*2)) ) : ( PINCON->PINSEL3 &= ~(0x3<<((PIN-16)*2)) )
#define GPIOPORT2SEL(PIN)              ( PINCON->PINSEL4 &= ~(0x3<<((PIN)*2)) )
#define GPIOPORT3SEL(PIN)              ( PINCON->PINSEL7 &= ~(0x3<<((PIN-16)*2)) )
#define GPIOPORT4SEL(PIN)              ( PINCON->PINSEL9 &= ~(0x3<<((PIN-16)*2)) )

// set GPIO direction
#define GPIODIR(PORT,PIN,MODE)           ( ((MODE) == INPUT) ? ( ((PORT)->REG_DIR) &= ~(0x1 << (PIN)) ) : ( ((PORT)->REG_DIR) |=  (0x1 << (PIN)) ) )
// GPIO clear, faster than digitalWrite
#define GPIOCLR(PORT,PIN)                ( (PORT)->REG_CLR ) |=  ( 0x1 << (PIN) )
// GPIO set, faster than digitalWrite
#define GPIOSET(PORT,PIN)                ( (PORT)->REG_SET ) |=  ( 0x1 << (PIN) )

// arduino style - GPIO output logic set/clear. On Code size's point of view this may be not a good idea(use function?)
#define _digitalRead(PORT,PIN)           ( ( (PORT)->REG_DATA & (0x1 << (PIN)) ) ? 1:0 )
#define _digitalWrite(PORT,PIN,VAL)      ( ((VAL) == LOW) ? ( ((PORT)->REG_CLR) |=  (0x1 << (PIN)) ) : ( ((PORT)->REG_SET) |=  (0x1 << (PIN)) ) )
#define _pinMode(PORT,PIN,MODE)          ( ((MODE) == INPUT) ? ( ((PORT)->REG_DIR) &= ~(0x1 << (PIN)) ) : ( ((PORT)->REG_DIR) |=  (0x1 << (PIN)) ) )

// arduino style - GPIO operation with port resolved
void pinMode(uint8_t PIN,uint8_t MODE);
void digitalWrite(uint8_t PIN,uint8_t VAL);
uint8_t digitalRead(uint8_t PIN);

void attachInterrupt(uint8_t ExternalInterruptNum, uint8_t pin, void (*userFunc)(void), uint8_t edgeState);
void detachInterrupt(uint8_t interruptNum);

void selectPinAsGPIO(uint8_t PIN);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);

// calibrate clock constants
#define CLKCONST_1  8000
#define CLKCONST_2  1
void _delay_ms(uint32_t t);


#endif /* PIN_H_ */
