/*
 * LCD.h
 *
 *  Created on: 30-May-2012
 *      Author: WPM
 *      V1.6
 */

#ifndef LCD_H_
#define LCD_H_

#define IDLE 			0  //0x0000
#define LEFT 			1  //0x0001
#define RIGHT 			2  //0x0010
#define UP				3  //0x0011
#define DOWN 			4  //0x0100
#define LEFT_UP			5  //0x0101
#define LEFT_DOWN		6  //0x0110
#define RIGHT_UP		7  //0x0111
#define RIGHT_DOWN		8  //0x1000
#define ZOOM_IN			9  //0x1001
#define ZOOM_OUT		10 //0x1010
#define PAN_LEFT		11 //0x1011
#define PAN_RIGHT		12 //0x1100
#define SELECT			13 //0x1101
#define LEFT_FAST		14 //0x1110
#define RIGHT_FAST		15 //0x1111

#define ONE				1  //0x0001
#define TWO 			2  //0x0010
#define THREE			3  //0x0011
#define FOUR 			4  //0x0100
#define FIVE			5  //0x0101
#define SIX				6  //0x0110
#define SEVEN			7  //0x0111
#define EIGHT			8  //0x1000
#define NINE			9  //0x1001
#define ZERO 			10 //0x1010
#define EXIT			11 //0x1011
#define BS				12 //0x1100

#define y1 15
#define y2 16
#define y3 17
#define y4 18

#define SCAN 			1  //0x0001
#define UNLOCKED		2  //0x0010
#define DENIED			3  //0x0011
#define WARNING 		4  //0x0100
#define ENGINEER		5  //0x0101
#define DESIGNER		6  //0x0110
#define ENTER_PWD		7  //0x0111
#define REENTER_PWD1		8  //0x1000
#define REENTER_PWD2		9  //0x1001
#define ACCESS			10 //0x1010
#define EXIT			11 //0x1011

#define LAST_PAGE 		3 //0x11
#define FIRST_PAGE		0 //0x11

#define MAIN			0 //Main Menu
#define SUB1			1 //Sub Menu 1
#define SYMBOL			2 //Sub Menu 1
#define RFID			3 //Sub Menu 1
#define GALLERY 		4 //Gallery Mode

void init_LCD();
int show_lcd(int cmd, int cmd2, int currentPage);

#endif /* LCD_H_ */
