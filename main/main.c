
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pin.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_nvic.h"
#include "PCD8544.h"
#include "time.h"
#include "LCD.h"
#include "motion.h"


/******************************************************************************
 **   Main Function  main()
 ******************************************************************************/
int main (void){


	init_LCD(); // Initialize LCD's GPIO
	init_motion();

/*********************** LCD Initialization ******************************/
    LCDInit(7,9,0,6,1,55);
    LCDclear();

    LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYALLON);
    Timer0_Wait(1000);

    LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
    LCDdisplay();

    int cmd = IDLE;
    int cmd2 = IDLE;
    int currentPage = 0;

//    sensor_debug();

/*********************** Command Processing ******************************/
    while(1) //Polling method to detect input
    {
    	currentPage=show_lcd(cmd, cmd2, currentPage);
    }
    return 0;
}
/******************************************************************************
 **                            End Of File
 ******************************************************************************/
