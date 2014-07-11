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
#include "multi_touch.h"
#include "symbol.h"
#include "single+multi.h"
#include "RFID.h"
#include "symbol.h"

static int menu = 0;
static int currentButton = 0;	//for sub menu
static int currentCursor = 0;	//for sub menu
static int currentState = 0;	//for RFID mode
static int zoom = 0;		//for gallery mode
static int currentX = 41;	//for gallery mode
static int currentY = 19;	//for gallery mode
static int currentWidth = 2;	//for gallery mode
static int currentHeight = 2;	//for gallery mode

#define x 2 //port 2
#define y 0 //port 0

//Function to initialize GPIO
void init_LCD()
{
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 1;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;

	int count;
	for(count=23; count<27; count++)
	{
		PinCfg.Pinnum = count;
		PINSEL_ConfigPin(&PinCfg);
	}

	PinCfg.Pinnum = 4;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 5;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 10;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(0,1<<23,0); // Set as input
	GPIO_SetDir(0,1<<24,0); // Set as input
	GPIO_SetDir(0,1<<25,0); // Set as input
	GPIO_SetDir(0,1<<26,0); // Set as input

	GPIO_SetDir(0,1<<4,0); // Set as input
	GPIO_SetDir(0,1<<5,0); // Set as input
	GPIO_SetDir(0,1<<10,0); // Set as input
	GPIO_SetDir(0,1<<11,0); // Set as input
}

// Function to read the value of a pin in port 2
uint8_t pin_Value(uint32_t input_pin_no)
{
	 return ((GPIO_ReadValue(0)>> input_pin_no) % 2);
}

// Function for input command reading (Motion Sensing)
int read_Input1(void)
{
	int input = IDLE;
	input	|= pin_Value(23); //MSB
	input <<= 1;
	input	|= pin_Value(24);
	input <<= 1;
	input	|= pin_Value(25);
	input <<= 1;
	input	|= pin_Value(26); //LSB
	//printf("cmd is %d\n",input);
	return input;
}

// Function for input command reading (RFID)
int read_Input2(void)
{
	int input = IDLE;
	input	|= pin_Value(11); //MSB
	input <<= 1;
	input	|= pin_Value(10);
	input <<= 1;
	input	|= pin_Value(5);
	input <<= 1;
	input	|= pin_Value(4); //LSB
	//printf("cmd2 is %d\n",input);
	return input;
}

/******************************************************************************
 *** FUNCTIONS FOR MAIN MENU
 ******************************************************************************/
//Function to compare input with command(Main)
int compare_InputMain(uint8_t cmd, int cPage)
{
	if(cmd==LEFT)
	{
		//printf("LEFT\n");
		if(cPage>=1) // min count is always 0
		{
			cPage--;
		}
	}
	else if(cmd==RIGHT)
	{
		//printf("RIGHT\n");
		if(cPage<=2) // max count is always 3
		{
			cPage++;
		}
	}
	else if(cmd==LEFT_FAST) //navigate to the first page
	{
		printf("LEFT_FAST\n");
		cPage=0;
	}
	else if(cmd==RIGHT_FAST)
	{
		printf("RIGHT_FAST\n"); //navigate to the last page
		cPage=3;
	}
	else if(cmd==SELECT)
	{
		printf("SELECT\n");
		LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED); //blink once
		Timer0_Wait(3000); //3sec
		LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
		LCDclear();
		LCDdisplay();

		switch(cPage)
		{
			case 0:		//page 1
				menu=SUB1;
				currentButton=0;
				currentCursor=0;
				printf("Entering SUB1\n");
				break;
			case 1:		//page 2
				menu=SYMBOL;
				currentButton=0;
				currentCursor=0;
				printf("Entering SYMBOL\n");
				break;
			case 2:		//page 3
				menu=RFID;
				currentState=0;
				printf("Entering RFID\n");
				break;
			case 3:		//page 4
				menu=GALLERY;
				currentX=41;
				currentY=19;
				currentWidth=2;
				currentHeight=2;
				zoom=0;
				printf("Entering GALLERY\n");
				break;
			default:
				break;
		}
	}
	else
	{
		printf("IDLE\n");
		cPage=cPage;
	}
	return cPage;
}

//Function to perform the sliding animation
void animation (int cPage, uint8_t cmd)
{
	if(cPage!=FIRST_PAGE && cmd==LEFT) //normal left
	{
		int count = 83;
		LCDclear();
		while(count>0)
		{
			LCDdrawline(count,44,count+3,41,BLACK); //arrow to the left
			LCDdrawline(count,44,count+3,47,BLACK); //arrow to the left
			LCDdisplay();
			count--;
		}
	}
	else if(cPage!=LAST_PAGE && cmd==RIGHT) //normal right
	{
		int count = 1;
		LCDclear();
		while(count<81)
		{
			LCDdrawline(count,41,count+3,44,BLACK); //arrow to the right
			LCDdrawline(count,47,count+3,44,BLACK); //arrow to the right
			LCDdisplay();
			count++;
		}
	}
	else if(cPage!=FIRST_PAGE && cmd==LEFT_FAST) //fast left
	{
		int count = 42;
		LCDclear();
		while(count>31)//count to 32
		{
			LCDdrawline(count,24,count+6,18,BLACK); //arrow to the left
			LCDdrawline(count,24,count+6,30,BLACK); //arrow to the left
			LCDdrawline(count-21,24,count-15,18,BLACK); //arrow to the left
			LCDdrawline(count-21,24,count-15,30,BLACK); //arrow to the left
			LCDdrawline(count+21,24,count+27,18,BLACK); //arrow to the left
			LCDdrawline(count+21,24,count+27,30,BLACK); //arrow to the left
			LCDdisplay();
			count--;
		}
	}
	else if(cPage!=LAST_PAGE && cmd==RIGHT_FAST) //fast right
	{
		int count = 36;
		LCDclear();
		while(count<47)//count to 46
		{
			LCDdrawline(count,18,count+6,24,BLACK); //arrow to the right
			LCDdrawline(count,30,count+6,24,BLACK); //arrow to the right
			LCDdrawline(count-21,18,count-15,24,BLACK); //arrow to the right
			LCDdrawline(count-21,30,count-15,24,BLACK); //arrow to the right
			LCDdrawline(count+21,18,count+27,24,BLACK); //arrow to the right
			LCDdrawline(count+21,30,count+27,24,BLACK); //arrow to the right
			LCDdisplay();
			count++;
		}
	}
	return;
}

//Function to display main menu
void display_Main(int page)
{
	LCDclear();
	switch(page)
	{
	case 0: 			//page 1
			LCDdrawstring(30,20,"FN 1");	// display page
			LCDdrawline(74,44,83,44,BLACK); // arrow to the right
			LCDdrawline(80,41,83,44,BLACK);
			LCDdrawline(80,47,83,44,BLACK);
			break;
	case 1: 			//page 2
			LCDdrawstring(26,15,"SYMBOL");	// display page
			LCDdrawstring(10,25,"RECOGNITION");
			LCDdrawline(74,44,83,44,BLACK); // arrow to the right
			LCDdrawline(80,41,83,44,BLACK);
			LCDdrawline(80,47,83,44,BLACK);
			LCDdrawline(1,44,10,44,BLACK); 	// arrow to the left
			LCDdrawline(1,44,4,41,BLACK);
			LCDdrawline(1,44,4,47,BLACK);
			break;
	case 2: 			//page 3
			LCDdrawstring(15,20,"RFID MODE");	// display page
			LCDdrawline(74,44,83,44,BLACK); // arrow to the right
			LCDdrawline(80,41,83,44,BLACK);
			LCDdrawline(80,47,83,44,BLACK);
			LCDdrawline(1,44,10,44,BLACK); 	// arrow to the left
			LCDdrawline(1,44,4,41,BLACK);
			LCDdrawline(1,44,4,47,BLACK);
			break;
	case 3:				//page 4
			LCDdrawstring(5,20,"GALLERY MODE");	// display page
			LCDdrawline(1,44,10,44,BLACK); 	// arrow to the left
			LCDdrawline(1,44,4,41,BLACK);
			LCDdrawline(1,44,4,47,BLACK);
			break;
	default:
			break;
	}
	LCDdisplay();
}


/******************************************************************************
 *** FUNCTIONS FOR SUB MENU 1
 ******************************************************************************/
//Function to compare input with command(Sub Menu)
int compare_InputSub(uint8_t cmd, int cCursor)
{
	int lastButton=3;

	if(cmd==UP)
	{
		//printf("UP\n");
		cCursor--;
		if(cCursor<0)
		{
			cCursor=0; // cannot scroll beyond the first button
		}
	}
	else if(cmd==DOWN)
	{
		//printf("DOWN\n");
		cCursor++;
		if(cCursor>lastButton)
		{
			cCursor=lastButton; //cannot scroll beyond the last button
		}
	}
	else if(cmd==SELECT)
	{
		//printf("SELECT\n");
		currentButton=cCursor;
		if(cCursor==lastButton) //select "BACK"
		{
			printf("exit SUB and go back to MAIN\n");
			menu=MAIN;
		}
	}
	else if(cmd==y1){
		cCursor=0;
	}
	else if(cmd==y2){
		cCursor=1;
	}
	else if(cmd==y3){
		cCursor=2;
	}
	else if(cmd==y4 && lastButton==3){
		cCursor=3;
	}
	else
	{
		//printf("IDLE\n");
		cCursor=cCursor;
	}
	return cCursor;
}

//Function to display sub menu 1
void display_Sub1(int cCursor)
{
	LCDclear();
	LCDdrawstring(7,4,"ON");
	LCDdrawstring(7,15,"OFF");
	LCDdrawstring(7,26,"AUTO");
	LCDdrawstring(7,37,"BACK");

	switch(currentButton)
	{
	case 0:
		LCDdrawrect(4, 2, 29, 11, BLACK);  //1st box
		break;
	case 1:
		LCDdrawrect(4, 13, 29, 11, BLACK); //2nd box
		break;
	case 2:
		LCDdrawrect(4, 24, 29, 11, BLACK); //3rd box
		break;
	case 3:
		LCDdrawrect(4, 35, 29, 11, BLACK); //4th box
		break;
	default:
		break;
	}
	display_Cursor(cCursor);
	LCDdisplay();
}

//Function to display cursor in all the sub menu
void display_Cursor(cCursor)
{
	switch(cCursor)
	{
	case 0:
		LCDdrawline(50, 7, 65, 7, BLACK); 	// arrow 1 to the left
		LCDdrawline(50, 7, 53, 4, BLACK);
		LCDdrawline(50, 7, 53, 10,BLACK);
		break;
	case 1:
		LCDdrawline(50, 18, 65, 18, BLACK); // arrow 2 to the left
		LCDdrawline(50, 18, 53, 15, BLACK);
		LCDdrawline(50, 18, 53, 21, BLACK);
		break;
	case 2:
		LCDdrawline(50, 29, 65, 29, BLACK); // arrow 3 to the left
		LCDdrawline(50, 29, 53, 26, BLACK);
		LCDdrawline(50, 29, 53, 32, BLACK);
		break;
	case 3:
		LCDdrawline(50, 40, 65, 40, BLACK);	// arrow 4 to the left
		LCDdrawline(50, 40, 53, 37, BLACK);
		LCDdrawline(50, 40, 53, 43, BLACK);
		break;
	default:
		break;
	}
}


/******************************************************************************
 *** FUNCTIONS FOR SYMBOL RECOGNITION
 ******************************************************************************/
//Function to compare input with command(Symbol Recognition)
void display_Instruction(int count)
{
	LCDclear();
	LCDdrawstring(13,10,"ENTER PWDS");
	switch(count)
	{
	case 1:
		LCDdrawstring(10,25,"X");
		break;
	case 2:
		LCDdrawstring(10,25,"X X");
		break;
	case 3:
		LCDdrawstring(10,25,"X X X");
		break;
	case 4:
		LCDdrawstring(10,25,"X X X X");
		break;
	case 5:
		LCDdrawstring(10,25,"X X X X X");
		break;
	case 6:
		LCDdrawstring(10,25,"X X X X X X");
		break;
	default:
			break;
	}
	//LCDdrawstring(10,20,"PLEASE DRAW");
	LCDdisplay();
}

//Function to display symbols
void display_Symbol(uint8_t cmd, int position)
{
	int posx=0;

	switch(position)
	{
	case 1:
		posx=10;
		//LCDfillrect(10, 17, 6, 8, 0); //clear that particular position
		break;
	case 2:
		posx=22;
		//LCDfillrect(16, 17, 6, 8, 0);
		break;
	case 3:
		posx=34;
		//LCDfillrect(22, 17, 6, 8, 0);
		break;
	case 4:
		posx=46;
		//LCDfillrect(28, 17, 6, 8, 0);
		break;
	case 5:
		posx=58;
		//LCDfillrect(34, 17, 6, 8, 0);
		break;
	case 6:
		posx=70;
		//LCDfillrect(40, 17, 6, 8, 0);
		break;
	default:
		break;
	}

	switch(cmd)
	{
	case 1:
		LCDdrawstring(posx,25,"1");
		break;
	case 2:
		LCDdrawstring(posx,25,"2");
		break;
	case 3:
		LCDdrawstring(posx,25,"3");
		break;
	case 4:
		LCDdrawstring(posx,25,"4");
		break;
	case 5:
		LCDdrawstring(posx,25,"5");
		break;
	case 6:
		LCDdrawstring(posx,25,"6");
		break;
	case 7:
		LCDdrawstring(posx,25,"7");
		break;
	case 8:
		LCDdrawstring(posx,25,"8");
		break;
	case 9:
		LCDdrawstring(posx,25,"9");
		break;
	case 10:
		LCDdrawstring(posx,25,"0");
		break;
	default:
		break;
	}
	LCDdisplay();
}


/******************************************************************************
 *** FUNCTIONS FOR RFID MODE
 ******************************************************************************/
//Function to compare input with command(RFID Mode)
int compare_InputRFID(uint8_t cmd2, int cState)
{
	if(cmd2==SCAN)
	{
		printf("SCAN\n");
		cState=0;
	}
	if(cmd2==UNLOCKED)
	{
		printf("UNLOCKED\n");
		cState=1;
	}
	if(cmd2==DENIED)
	{
		printf("DENIED\n");
		cState=2;
	}
	if(cmd2==WARNING)
	{
		printf("WARNING\n");
		cState=3;
	}
	if(cmd2==ENGINEER)
	{
		printf("ENGINEER\n");
		cState=4;
	}
	if(cmd2==DESIGNER)
	{
		printf("DESIGNER\n");
		cState=5;
	}
	if(cmd2==ENTER_PWD)
	{
		printf("ENTER_PWD\n");
		cState=6;
	}
	if(cmd2==REENTER_PWD1)
	{
		printf("REENTER_PWD1\n");
		cState=7;
	}
	if(cmd2==REENTER_PWD2)
	{
		printf("REENTER_PWD2\n");
		cState=8;
	}
	if(cmd2==ACCESS)
	{
		printf("ACCESS\n");
		cState=9;
	}
	if(cmd2==EXIT)
	{
		printf("exit SUB and go back to MAIN\n");
		menu=MAIN;
	}
	else
	{
		printf("IDLE\n");
		cState=cState;
	}
	return cState;
}

//Function to display states in RFID mode
void display_RFID(cState)
{
	LCDclear();
	switch(cState)
	{
	case 0:
			LCDdrawstring(8,15,"PLEASE SCAN");
		    LCDdrawstring(12,25,"YOUR CARD");
			break;
	case 1:
			LCDdrawstring(23,15,"SYSTEM");
		    LCDdrawstring(17,25,"UNLOCKED");;
			break;
	case 2:
			LCDdrawstring(3,20,"ACCESS DENIED");
			break;
	case 3:
			LCDdrawstring(19,20,"WARNING!");
			LCDdisplay();
			LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED);  //blinking screen
			Timer0_Wait(500);
			LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
			Timer0_Wait(500);
			LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED);
			Timer0_Wait(500);
			LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
			Timer0_Wait(500);
			LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED);
			Timer0_Wait(500);
			LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
			menu=MAIN;
			printf("exit SUB and go back to MAIN\n");
			break;
	case 4:
			LCDdrawstring(19,15,"WELCOME,");
		    LCDdrawstring(17,25,"ENGINEER!");
			break;
	case 5:
			LCDdrawstring(19,15,"WELCOME,");
		    LCDdrawstring(17,25,"DESIGNER!");
			break;
	case 6:
			LCDdrawstring(6,15,"PLEASE ENTER");
		    LCDdrawstring(0,25,"YOUR PASSWORDS");
			break;
	case 7:
			LCDdrawstring(26,0,"WRONG!");
		    LCDdrawline(24,9,61,9,BLACK);
		    LCDdrawstring(22,15,"REENTER");
		    LCDdrawstring(0,25,"YOUR PASSWORDS");
		    LCDdrawstring(0,40,"TRIAL LEFT:2");
			break;
	case 8:
			LCDdrawstring(7,0,"WRONG AGAIN!");
		    LCDdrawline(5,9,77,9,BLACK);
		    LCDdrawstring(22,15,"REENTER");
		    LCDdrawstring(0,25,"YOUR PASSWORDS");
		    LCDdrawstring(0,40,"TRIAL LEFT:1");
			break;
	case 9:
			LCDdrawstring(0,20,"ACCESS GRANTED");
			break;
	default:
			break;
	}
	LCDdisplay();
}

/******************************************************************************
 *** FUNCTIONS FOR GALLERY MODE
 ******************************************************************************/
//Function to compare input with command(Gallery Mode)
void compare_InputGal(uint8_t cmd)
{
	//printf("%d\n",cmd);
	if(cmd==LEFT)
	{
		printf("LEFT\n");
		if(currentX>=1) //cannot move beyond the left frame (min count 0)
		{
			currentX--;
		}
	}
	else if(cmd==RIGHT)
	{
		printf("RIGHT\n");
		if((currentWidth+currentX)<=82) //cannot move beyond the right frame (max count 83)
		{
			currentX++;
		}
	}
	else if(cmd==UP)
	{
		printf("UP\n");
		if(currentY>=1) //cannot move beyond the upper frame (min count 0)
		{
			currentY--;
		}
	}
	else if(cmd==DOWN)
	{
		printf("DOWN\n");
		if((currentY+currentHeight)<=38) //cannot move beyond the lower frame (max count 39)
		{
			currentY++;
		}
	}
	else if(cmd==LEFT_UP)
	{
		if(currentX>=1 && currentY>=1){
			currentX--;
			currentY--;
		}
	}
	else if(cmd==LEFT_DOWN)
	{
		if(currentX>=1 && (currentY+currentHeight)<=38){
			currentX--;
			currentY++;
		}
	}
	else if(cmd==RIGHT_UP)
	{
		if((currentWidth+currentX)<=82 && currentY>=1)
		{
			currentX++;
			currentY--;
		}
	}
	else if(cmd==RIGHT_DOWN)
	{
		if((currentWidth+currentX)<=82 && (currentY+currentHeight)<=38)
		{
			currentX++;
			currentY++;
		}
	}
	else if(cmd==ZOOM_IN)
	{
		//printf("ZOOM_IN\n");
		zoom++;
		currentX = currentX-1;
		currentY = currentY-1;
		currentWidth = 2+(zoom*2);
		currentHeight = 2+(zoom*2);
		/*printf("zoom: %d\n",zoom);
		printf("cX: %d\n",currentX);
		printf("cY: %d\n",currentY);
		printf("cW: %d\n",currentWidth);
		printf("cH: %d\n",currentHeight);*/
		if( (currentX<0) || ((currentY+currentHeight)>39) || (currentY<0) || ((currentWidth+currentX)>83))
		{
			printf("Reached frame\n");	 //if the object reaches the frame, it will not zoom and stay as it is
			zoom--;
			currentX = currentX+1;
			currentY = currentY+1;
			currentWidth = 2+((zoom)*2);
			currentHeight = 2+((zoom)*2);
			/*printf("zoom: %d\n",zoom);
			printf("cX: %d\n",currentX);
			printf("cY: %d\n",currentY);
			printf("cW: %d\n",currentWidth);
			printf("cH: %d\n",currentHeight);*/
		}
	}
	else if(cmd==ZOOM_OUT)
	{
		//printf("ZOOM_OUT\n");
		if(zoom>=1)
		{
			zoom--;
			currentX = currentX+1;
			currentY = currentY+1;
			currentWidth = 2+(zoom*2);
			currentHeight = 2+(zoom*2);
		}
	}
	else
	{
		//printf("IDLE\n");
		currentX=currentX;
		currentY=currentY;
	}
}

//Function to display current location and size of the object in Gallery mode
void display_Gal(int tnumber)
{
	LCDclear();
	if(tnumber==0){
		LCDdrawline(80,41,83,44,BLACK);	// arrow to the right
		LCDdrawline(80,47,83,44,BLACK);
	}
	else if(tnumber==2){
		LCDdrawline(1,44,4,41,BLACK);	// arrow to the left
		LCDdrawline(1,44,4,47,BLACK);
	}
	else{
		LCDdrawline(80,41,83,44,BLACK);	// arrow to the right
		LCDdrawline(80,47,83,44,BLACK);
		LCDdrawline(1,44,4,41,BLACK);	// arrow to the left
		LCDdrawline(1,44,4,47,BLACK);
	}

	switch(tnumber){
	case 0:
		LCDdrawstring(26,40,"TOOL 1");
		break;
	case 1:
		LCDdrawstring(26,40,"TOOL 2");
		break;
	case 2:
		LCDdrawstring(26,40,"TOOL 3");
		break;
	default:
		break;
	}
	LCDfillrect(currentX, currentY, currentWidth, currentHeight, BLACK);
	LCDdisplay();
}


int gesture(){
	int swipe=FALSE;
	int cmd=IDLE;

	if(swipe==FALSE){
		cmd=select();
		if(cmd==IDLE){
			swipe=TRUE;
		}
	}

	if(swipe==TRUE){
	cmd = scan(x, 0, 5);
	swipe=FALSE;
	}

	return cmd;
}


/******************************************************************************
 *** MAIN FUNCTION FOR LCD DISPLAY
 ******************************************************************************/
int show_lcd(int cmd, int cmd2, int currentPage){
int count=0;
int number=0;
int tnumber=0;
int swipe=FALSE;
int difference=0;

	while(menu==MAIN) //in main menu
	{	cmd=IDLE;
		//printf("in MAIN now\n");
		display_Main(currentPage);
		cmd=get_command();
		//printf("enter cmd \n");
		//scanf("%d",&cmd);
		//Timer0_Wait(1000);
		//****gesture****
/*		if(swipe==FALSE){
			cmd=select();
			if(cmd==IDLE){
				swipe=TRUE;
			}
		}
		if(swipe==TRUE){
		cmd = scan(x, 0, 5);
		swipe=FALSE;
		}
*/
		//***************
		animation(currentPage, cmd);
		currentPage = compare_InputMain(cmd,currentPage);
	}
	while(menu==SUB1) //in sub menu 1
	{
		cmd=IDLE;
		//printf("in SUB 1 now\n");
		display_Sub1(currentCursor); //display current selected button
		cmd=get_command();
		//printf("enter cmd \n");
		//scanf("%d",&cmd);
		//Timer0_Wait(1000);
		//****gesture****
/*		if(swipe==FALSE){
			cmd=select();
			if(cmd==IDLE){
				swipe=TRUE;
			}
		}
		if(swipe==TRUE){
		cmd = ultra_fast_scan();
		swipe=FALSE;
		}
*/
		//***************
		currentCursor = compare_InputSub(cmd,currentCursor); //current cursor
		display_Sub1(currentCursor);
		Timer0_Wait(1000);

	}
	while(menu==SYMBOL) //in symbol recognition menu (passwords)
	{
		cmd=IDLE;
		//printf("in SYMBOL now\n");
		display_Instruction(number); //display instruction
		//printf("enter cmd \n");
		//scanf("%d",&cmd);
		recognise_number();
		cmd=command_symbol();

		if(cmd==EXIT){
			menu=MAIN;
			number=0;
		}
		else if(cmd==BS && number>0){				//backspace
			number--;								//number is used to count how many numbers has the user input
		}
		else if(cmd!=IDLE && cmd!=BS && number<6){ 	//not idle and not backspace
			number++;
			display_Symbol(cmd,number);
			Timer0_Wait(1000);						//display the number for 1 sec
		}
	}
	while(menu==RFID)
	{
		RFID_scan();
		/*
		//printf("in RFID now\n");
		display_RFID(currentState); //display current state
		//Timer0_Wait(3000);
		cmd2=read_Input2();
		currentState = compare_InputRFID(cmd2,currentState); //current state
		*/
	}
	while(menu==GALLERY)//in gallery mode
	{	cmd=IDLE;
		//printf("in GALLERY now\n");
		display_Gal(tnumber); //display current location and size of the shape
		//printf("enter cmd \n");
		//scanf("%d",&cmd);


		//printf("command received is %d\n", cmd);
		cmd=get_command();
		difference=get_difference();
		count=0;
		while(count<=difference && difference!=0){
			compare_InputGal(cmd);
			display_Gal(tnumber);
			count++;
		}
		difference=0;
		compare_InputGal(cmd);

		if(cmd==SELECT){
			printf("exit GALLERY and go back to MAIN\n");
			tnumber=0;
			menu=MAIN;
		}
		else if(cmd==PAN_LEFT && tnumber>0){
			tnumber--;
		}
		else if(cmd==PAN_RIGHT && tnumber<2){
			tnumber++;
		}

	/*	difference=multitouch();

		if(difference!=0){
			cmd=command();
		}

		if(difference<0){
			difference=difference*-1;
		}
		count=0;
		while(count<=difference && difference!=0){
			compare_InputGal(cmd);
			display_Gal();
			count++;
		}
		difference=0;*/
	}
	return currentPage;
}
