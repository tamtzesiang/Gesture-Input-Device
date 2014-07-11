/*
=================================================================================
 Name        : pin.c
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

#include "pin.h"
//#include "../inc/pin.h"

// LPC1769 port resolving
// index to this array to get a correct port pointer
// This is a PRIVATE definition (cannot be in pin.h - multiple def error when include pin.h)
const uint8_t digital_pin_to_GPIO_port[] =
{
        /*  PORT     LPC_MAP   PIN_MAP   */
                GPIO0, /* p0.0        0      */
                GPIO0, /* p0.1        1      */
                GPIO0, /* p0.2        2      */
                GPIO0, /* p0.3        3      */
                GPIO0, /* p0.4        4      */
                GPIO0, /* p0.5        5      */
                GPIO0, /* p0.6        6      */
                GPIO0, /* p0.7        7      */
                GPIO0, /* p0.8        8      */
                GPIO0, /* p0.9        9      */
                GPIO0, /* p0.10       10     */
                GPIO0, /* p0.11       11     */
                GPIO0, /* p0.15       12     */
                GPIO0, /* p0.16       13     */
                GPIO0, /* p0.17       14     */
                GPIO0, /* p0.18       15     */
                GPIO0, /* p0.19       16     */
                GPIO0, /* p0.20       17     */
                GPIO0, /* p0.21       18     */
                GPIO0, /* p0.22       19     */
                GPIO0, /* p0.23       20     */
                GPIO0, /* p0.24       21     */
                GPIO0, /* p0.25       22     */
                GPIO0, /* p0.26       23     */
                GPIO0, /* p0.27       24     */
                GPIO0, /* p0.28       25     */
                GPIO0, /* p0.29       26     */
                GPIO0, /* p0.30       27     */

                /*---------------------------*/

                GPIO1, /* p1.0        28     */
                GPIO1, /* p1.1        29     */
                GPIO1, /* p1.4        30     */
                GPIO1, /* p1.8        31     */
                GPIO1, /* p1.9        32     */
                GPIO1, /* p1.10       33     */
                GPIO1, /* p1.14       34     */
                GPIO1, /* p1.15       35     */
                GPIO1, /* p1.16       36     */
                GPIO1, /* p1.17       37     */
                GPIO1, /* p1.18       38     */
                GPIO1, /* p1.19       39     */
                GPIO1, /* p1.20       40     */
                GPIO1, /* p1.21       41     */
                GPIO1, /* p1.22       42     */
                GPIO1, /* p1.23       43     */
                GPIO1, /* p1.24       44     */
                GPIO1, /* p1.25       45     */
                GPIO1, /* p1.26       46     */
                GPIO1, /* p1.27       47     */
                GPIO1, /* p1.28       48     */
                GPIO1, /* p1.29       49     */
                GPIO1, /* p1.30       50     */
                GPIO1, /* p1.31       51     */

                /*---------------------------*/

                GPIO2, /* p2.0        52     */
                GPIO2, /* p2.1        53     */
                GPIO2, /* p2.2        54     */
                GPIO2, /* p2.3        55     */
                GPIO2, /* p2.4        56     */
                GPIO2, /* p2.5        57     */
                GPIO2, /* p2.6        58     */
                GPIO2, /* p2.7        59     */
                GPIO2, /* p2.8        60     */
                GPIO2, /* p2.9        61     */
                GPIO2, /* p2.10       62     */
                GPIO2, /* p2.11       63     */
                GPIO2, /* p2.12       64     */
                GPIO2, /* p2.13       65     */

                /*---------------------------*/

                GPIO3, /* p3.25       66     */
                GPIO3, /* p3.26       67     */

                /*---------------------------*/

                GPIO4, /* p4.28       68     */
                GPIO4, /* p4.29       69     */
};

const uint8_t digital_pin_to_GPIO_pin[] =
{
        /*  PORT     LPC_MAP   PIN_MAP   */
                (0), /* p0.0        0      */
                (1), /* p0.1        1      */
                (2), /* p0.2        2      */
                (3), /* p0.3        3      */
                (4), /* p0.4        4      */
                (5), /* p0.5        5      */
                (6), /* p0.6        6      */
                (7), /* p0.7        7      */
                (8), /* p0.8        8      */
                (9), /* p0.9        9      */
                (10), /* p0.10       10     */
                (11), /* p0.11       11     */
                (15), /* p0.15       12     */
                (16), /* p0.16       13     */
                (17), /* p0.17       14     */
                (18), /* p0.18       15     */
                (19), /* p0.19       16     */
                (20), /* p0.20       17     */
                (21), /* p0.21       18     */
                (22), /* p0.22       19     */
                (23), /* p0.23       20     */
                (24), /* p0.24       21     */
                (25), /* p0.25       22     */
                (26), /* p0.26       23     */
                (27), /* p0.27       24     */
                (28), /* p0.28       25     */
                (29), /* p0.29       26     */
                (30), /* p0.30       27     */

                /*---------------------------*/

                (0), /* p1.0        28     */
                (1), /* p1.1        29     */
                (4), /* p1.4        30     */
                (8), /* p1.8        31     */
                (9), /* p1.9        32     */
                (10), /* p1.10       33     */
                (14), /* p1.14       34     */
                (15), /* p1.15       35     */
                (16), /* p1.16       36     */
                (17), /* p1.17       37     */
                (18), /* p1.18       38     */
                (19), /* p1.19       39     */
                (20), /* p1.20       40     */
                (21), /* p1.21       41     */
                (22), /* p1.22       42     */
                (23), /* p1.23       43     */
                (24), /* p1.24       44     */
                (25), /* p1.25       45     */
                (26), /* p1.26       46     */
                (27), /* p1.27       47     */
                (28), /* p1.28       48     */
                (29), /* p1.29       49     */
                (30), /* p1.30       50     */
                (31), /* p1.31       51     */

                /*---------------------------*/

                (0), /* p2.0        52     */
                (1), /* p2.1        53     */
                (2), /* p2.2        54     */
                (3), /* p2.3        55     */
                (4), /* p2.4        56     */
                (5), /* p2.5        57     */
                (6), /* p2.6        58     */
                (7), /* p2.7        59     */
                (8), /* p2.8        60     */
                (9), /* p2.9        61     */
                (10), /* p2.10       62     */
                (11), /* p2.11       63     */
                (12), /* p2.12       64     */
                (13), /* p2.13       65     */

                /*---------------------------*/

                (25), /* p3.25       66     */
                (26), /* p3.26       67     */

                /*---------------------------*/

                (28), /* p4.28       68     */
                (29), /* p4.29       69     */
};


static gpio_type * portPointer[] =
{
                LPC_GPIO0,
                LPC_GPIO1,
                LPC_GPIO2,
                LPC_GPIO3
};


#define digitalPinToGPIOPort(P) ( *(digital_pin_to_GPIO_port + (P)) )
#define digitalPinToGPIOPin(P)  ( *(digital_pin_to_GPIO_pin + (P)) )
#define getGPIOPortPointer(P)       ( *(portPointer + digitalPinToGPIOPort(P)) )

// these are convenient but slow, wherever possible use macro (with port)
/**********************************************************************
 * @brief       set pin direction
 *
 * @param       PIN       selected pin (see pin mapping above)
 * @param       MODE      direction mode, should be
 *                        - INPUT
 *                        - OUTPUT
 * @return      none
 **********************************************************************/
void pinMode(uint8_t PIN,uint8_t MODE)
{
        _pinMode(getGPIOPortPointer(PIN), digitalPinToGPIOPin(PIN), MODE);
}

/**********************************************************************
 * @brief       write a desired logic to output pin
 *
 * @param       PIN       selected pin (see pin mapping above)
 * @param       VAL       logic valuee, should be
 *                        - HIGH
 *                        - LOW
 * @return      none
 **********************************************************************/
void digitalWrite(uint8_t PIN,uint8_t VAL)
{
        _digitalWrite(getGPIOPortPointer(PIN), digitalPinToGPIOPin(PIN), VAL);
}

/**********************************************************************
 * @brief       read logic level on input pin
 *
 * @param       PIN       selected pin (see pin mapping above)
 * @return      uint8_t   logic level
 **********************************************************************/
uint8_t digitalRead(uint8_t PIN)
{
        return (uint8_t) _digitalRead(getGPIOPortPointer(PIN),digitalPinToGPIOPin(PIN));
}

/**********************************************************************
 * @brief       select pin as GPIO
 *
 * @param       PIN       selected pin (see pin mapping above)
 * @return      none
 **********************************************************************/
void selectPinAsGPIO(uint8_t PIN)
{
        uint8_t gpio_port = digitalPinToGPIOPort(PIN);
        uint8_t gpio_pin = digitalPinToGPIOPin(PIN);

        switch (gpio_port)
        {
        case 0:
                GPIOPORT0SEL(gpio_pin);
                break;
        case 1:
                GPIOPORT1SEL(gpio_pin);
                break;
        case 2:
                GPIOPORT2SEL(gpio_pin);
                break;
        case 3:
                GPIOPORT3SEL(gpio_pin);
                break;
        case 4:
                GPIOPORT4SEL(gpio_pin);
                break;
        default:
                break;
        }

}

/**********************************************************************
 * @brief       bitbang serial shift out on select GPIO pin. Data rate
 *              is defined by CPU clk speed and CLKCONST_2. Calibrate
 *              these value for your need on target platform.
 *
 * @param       dataPin       selected pin (see pin mapping above)
 * @param       clockPin      a simulating clk output by pull pin HIGH
 *                            and LOW periodicly
 * @param       bitOrder      select MSB or LSB first mode, should be:
 *                            - LSBFIRST
 *                            - MSBFIRST
 * @param       val           byte to shift out
 * @return      none
 **********************************************************************/
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
        uint8_t i;
        uint32_t j;

        for (i = 0; i < 8; i++)  {
                if (bitOrder == LSBFIRST)
                        digitalWrite(dataPin, !!(val & (1 << i)));
                else
                        digitalWrite(dataPin, !!(val & (1 << (7 - i))));

                digitalWrite(clockPin, HIGH);
                for (j = CLKCONST_2; j > 0; j--); // clock speed, anyone? (LCD Max CLK input: 4MHz)
                digitalWrite(clockPin, LOW);
        }
}

/**********************************************************************
 * @brief       roughly calibrated spin delay
 *
 * @param       t       delay in ms
 * @return      none
 **********************************************************************/
void _delay_ms(uint32_t t)// roughly calibrated spin delay
{
        uint32_t nCount = 0;
        while (t != 0)
        {
                nCount = CLKCONST_1;
                while(nCount != 0)
                        nCount--;
                t--;
        }
}

/****************************** NXP APIs for GPIO interrupt: ***************************/
/*********************************************************************//**
 * @brief       Enable GPIO interrupt (just used for P0.0-P0.30, P2.0-P2.13)
 * @param[in]   portNum     Port number to read value, should be: 0 or 2
 * @param[in]   bitValue    Value that contains all bits on GPIO to enable,
 *                          in range from 0 to 0xFFFFFFFF.
 * @param[in]   edgeState   state of edge, should be:
 *                          - 0: Rising edge
 *                          - 1: Falling edge
 * @return      None
 **********************************************************************/
static void GPIO_IntCmd(uint8_t portNum, uint32_t bitValue, uint8_t edgeState)
{
        if((portNum == 0)&&(edgeState == 0))
                LPC_GPIOINT->IO0IntEnR = bitValue;
        else if ((portNum == 2)&&(edgeState == 0))
                LPC_GPIOINT->IO2IntEnR = bitValue;
        else if ((portNum == 0)&&(edgeState == 1))
                LPC_GPIOINT->IO0IntEnF = bitValue;
        else if ((portNum == 2)&&(edgeState == 1))
                LPC_GPIOINT->IO2IntEnF = bitValue;
        else
                //Error
                while(1);
}

/*********************************************************************//**
 * @brief       Get GPIO Interrupt Status (just used for P0.0-P0.30, P2.0-P2.13)
 * @param[in]   portNum     Port number to read value, should be: 0 or 2
 * @param[in]   pinNum      Pin number, should be: 0..30(with port 0) and 0..13
 *                          (with port 2)
 * @param[in]   edgeState   state of edge, should be:
 *                          - 0: Rising edge
 *                          - 1: Falling edge
 * @return      Bool        should be:
 *                          - true: Interrupt has been generated due to a rising
 *                                                              edge on P0.0
 *                          - false: A rising edge has not been detected on P0.0
 **********************************************************************/
static bool GPIO_GetIntStatus(uint8_t portNum, uint32_t pinNum, uint8_t edgeState)
{
        if((portNum == 0) && (edgeState == 0))//Rising Edge
                return ((bool)(((LPC_GPIOINT->IO0IntStatR)>>pinNum)& 0x1));
        else if ((portNum == 2) && (edgeState == 0))
                return ((bool)(((LPC_GPIOINT->IO2IntStatR)>>pinNum)& 0x1));
        else if ((portNum == 0) && (edgeState == 1))//Falling Edge
                return ((bool)(((LPC_GPIOINT->IO0IntStatF)>>pinNum)& 0x1));
        else if ((portNum == 2) && (edgeState == 1))
                return ((bool)(((LPC_GPIOINT->IO2IntStatF)>>pinNum)& 0x1));
        else
                //Error
                while(1);
}
/**********************************************************************
 * @brief       Clear GPIO interrupt (just used for P0.0-P0.30, P2.0-P2.13)
 * @param[in]   portNum     Port number to read value, should be: 0 or 2
 * @param[in]   bitValue    Value that contains all bits on GPIO to enable,
 *                          in range from 0 to 0xFFFFFFFF.
 * @return      None
 **********************************************************************/
static void GPIO_ClearInt(uint8_t portNum, uint32_t bitValue)
{
        if(portNum == 0)
                LPC_GPIOINT->IO0IntClr = bitValue;
        else if (portNum == 2)
                LPC_GPIOINT->IO2IntClr = bitValue;
        else
                //Invalid portNum
                while(1);
}

/*********************************************************************************
 * Our definitions for Arduino style interrupts
 *********************************************************************************/

volatile static voidFuncPtr intFunc[EXTERNAL_NUM_INTERRUPTS];
volatile static uint8_t interruptState[EXTERNAL_NUM_INTERRUPTS][2];

static bool isGPIOPinInterrupted(uint8_t PIN,uint8_t edgeState)
{
        uint8_t gpio_port = digitalPinToGPIOPort(PIN);
        uint8_t gpio_pin = digitalPinToGPIOPin(PIN);
        return GPIO_GetIntStatus(gpio_port,gpio_pin,edgeState);
}

static void clearGPIOInterrupted(uint8_t PIN)
{
        uint8_t gpio_port = digitalPinToGPIOPort(PIN);
        uint8_t gpio_pin = digitalPinToGPIOPin(PIN);
        GPIO_ClearInt(gpio_port,(1<<gpio_pin));
}

/***********************************************************************************************
 * @brief        Attach GPIO to interrupt handler EINT3 (just used for P0.0-P0.30, P2.0-P2.13)
 *
 * @param        pin                    pin which interrupt attached to
 * @param        ExternalInterruptNum   external interrupt number
 * @param        userFunc               pointer to user defined irq
 * @param        edgeState              interrupt select mode:
 *                                      - 0: Rising edge
 *                                      - 1: Falling edge
 * @return       none
 *
 * @NOTE:        - This version only supports external interrupt on EINT3 vector,
 *               - There is a difference with arduino attachInterrupt() : provide gpio pin number
 ***********************************************************************************************/
void attachInterrupt(uint8_t ExternalInterruptNum, uint8_t pin, void (*userFunc)(void), uint8_t edgeState)
{
        if(ExternalInterruptNum < EXTERNAL_NUM_INTERRUPTS)
        {
                uint8_t gpio_port = digitalPinToGPIOPort(pin);
                uint8_t gpio_pin = digitalPinToGPIOPin(pin);
                GPIO_IntCmd(gpio_port,(1<<gpio_pin),edgeState);
                //intFunc[interruptNum] = userFunc;

                // only EINT3 currently supported
                intFunc[3] = userFunc;
                interruptState[3][0] = pin;
                interruptState[3][1] = edgeState;
                NVIC_EnableIRQ(EINT3_IRQn);
        }
}
/***********************************************************************************************
 * @brief        detach GPIO interrupt from EINT3 ISR
 *
 * @param        ExternalInterruptNum   external interrupt number
 * @return       none
 *
 * @NOTE:        - This version only supports external interrupt on EINT3 vector,
 ***********************************************************************************************/
void detachInterrupt(uint8_t ExternalInterruptNum)
{
        if(ExternalInterruptNum < EXTERNAL_NUM_INTERRUPTS)
        {
                // only EINT3 currently supported
                NVIC_DisableIRQ(EINT3_IRQn);
                //intFunc[interruptNum] = 0;
                intFunc[3] = 0;
        }
}

/***********************************************************************************************
 * @brief        entrance for GPIO interrupt (EINT3 ISR). This routine fired up whenever an event
 *               happened at attached GPIO. It then will select the appropriate user defined ISR
 *               for that interrupt.
 *
 * @param        none
 * @return       none
 *
 * @NOTE:        - This version only supports external interrupt on EINT3 vector,
 ***********************************************************************************************/
void EINT3_IRQHandler(void)
{
        if(intFunc[EXTERNAL_INT_3] && isGPIOPinInterrupted(interruptState[3][0],interruptState[3][1]))
        {
                clearGPIOInterrupted(interruptState[3][0]);
                intFunc[EXTERNAL_INT_3]();
        }
}
