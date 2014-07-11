
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "LPC17xx.h"
#include "stdio.h"
#include <string.h>
#include "time.h"
#include "LCD.h"
#include "RFID.h"


uint8_t len = 0;					//Declaring the length of received ID from RFID
uint8_t line[10];					//Array to store ID received from RFID which consists of 10 characters
uint8_t data = 0;					//Declaring variable 'data' which is used to store one byte(total 8 bytes) from UART buffer
uint8_t counter = 0;				//Increment counter which is used to check if the ID's is in the database

char user_x[10]= "0F02F2E24C"; 		//Stored user ID (this declaration and function that uses this declaration is not significant in this system, just if we need to expand the system in the future)
char x1[] = {"E"};					//Declaration which we will use to check the '7th' character of the User ID
char x2[] = {"D"};					//Declaration which we will use to check the '7th' character of the User ID

char x0F02F2E24C[];					//to used as string compare in compare_password function. the 'x' (or any other alphabet) is needed in front because variable cannot start from a number
char x0F02F2DD3E[];					//to used as string compare in compare_password function. the 'x' (or any other alphabet) is needed in front because variable cannot start from a number

int *fptr;							//Declaring a pointer which will be used in structure
int counter_denied =0;				//Declaring a counter that counts how many times a person tapped

char password[10];					//Declaring an array which will be used to stored the password

static int GMThour;					//variable to store the GMT time
static int hour;					//To store current time
static int minute;					//To store current minute
static int day;						//To store current day
static int oldmonth;				//To store current oldmonth
static int month;					//To store current month

static int last_hour_a0F02F2E24C;   	//To store the last access hour by a0F02F2E24C
static int last_minute_a0F02F2E24C;		//To store the last access minute by a0F02F2E24C
static int last_day_a0F02F2E24C;		//To store the last access day by a0F02F2E24C
static int last_month_a0F02F2E24C;		//To store the last access month by a0F02F2E24C

static int last_hour_a0F02F2DD3E;		//To store the last access hour by a0F02F2DD3E
static int last_minute_a0F02F2DD3E;		//To store the last access minute by a0F02F2DD3E
static int last_day_a0F02F2DD3E;		//To store the last access day by a0F02F2DD3E
static int last_month_a0F02F2DD3E;		//To store the last access month by a0F02F2DD3E

static char *commandList[] =			//The database which store which user is able to access
	{
		"0F02F2E24C",
		"0F02F2DD3E"
	};

	//initialise P0.11 as UART Receive. Because RFID reader has Serial Output(to LPC)
	void pinsel_uart2(void)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 1;  		//Select function as Receive, RXD2
		PinCfg.Portnum = 0;
		PinCfg.Pinnum = 10;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 11;
		PINSEL_ConfigPin(&PinCfg);
	}

	//initialise GPIO P2.13
	void init_GPIO_2_13(void)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0; 		//Select function as GPIO
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 2;
		PinCfg.Pinnum = 13;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(2, 1<<13, 1); 	//Set as output, '1' to /ENABLE of RFID to pull it to active low
	}
/*
	//initialise GPIO P0.2
	void init_GPIO_0_2(void)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0; 		//Select function as GPIO
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 0;
		PinCfg.Pinnum = 2;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(0, 1<<2, 1); 	//Set as output, '0' or '1' to send LCD screen
	}

	//initialise GPIO P0.3
	void init_GPIO_0_3(void)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0; 		//Select function as GPIO
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 0;
		PinCfg.Pinnum = 3;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(0, 1<<3, 1); 	//Set as output, '0' or '1' to send LCD screen
	}

	//initialise GPIO P0.4
	void init_GPIO_0_4(void)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0; 		//Select function as GPIO
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 0;
		PinCfg.Pinnum = 4;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(0, 1<<4, 1); 	//Set as output, '0' or '1' to send LCD screen
	}

	//Initialise GPIO P2.12
	void init_GPIO_2_12(void)
	{
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 0; 		//Select function as GPIO
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 2;
		PinCfg.Pinnum = 12;
		PINSEL_ConfigPin(&PinCfg);
		GPIO_SetDir(2, 1<<12, 1); 	//Set as output, '0' or '1' to send LCD screen
	}

	*/
	//Configuring initialisation of UART
	void init_uart2(void)
	{
		UART_CFG_Type uartCfg;
		uartCfg.Baud_rate = 2400;  			//RFID reader with baud rate of 2400bps
		uartCfg.Databits = UART_DATABIT_8;  //RFID reader with 8 data bits
		uartCfg.Parity = UART_PARITY_NONE;  //RFID reader with no parity
		uartCfg.Stopbits = UART_STOPBIT_1;	//RFID reader with one stop bit

		pinsel_uart2();						//Pin select for UART3
		UART_Init(LPC_UART2, &uartCfg);		//Supply power & setup working parts for UART3
		UART_TxCmd(LPC_UART2, ENABLE);		//Enable transmit for UART3
	}

	//compareID function - to compare how many characters are same with the master ID card(user_x)
	int compareID(i, numberOfSame)
	{
		for(i=0;i<10;i++)
		{									//counting number of same of character with the master ID card is not necessary in this subsystem
			if (line[i] == user_x[i])		//I implemented it because this comparing ID method can be scale up if necessary
			{								//It really depends on how the programmer codes the system
				numberOfSame += 1;
			}
		}


		if(numberOfSame > 4)   				//So in our case, if there are more than 4 characters which is the same as the User ID, it will go into this condition
		{
			numberOfSame = 0;				//reseting numberOfSame to '0'

			if(line[6] == x1[0])			//if the '7th' character of the unique ID is 'E' as declared above, it will return '1' to the switch case below
			{
				return 1;
			}

			if(line[6] == x2[0])			//if the '7th' character of the unique ID is 'D' as declared above, it will return '2' to the switch case below
			{
				return 2;
			}

		}

		else								//Our system wont go into this 'else' case, just if the unique ID number is less than '4' (not possible in our case)
		{									//As I said, this comparing method just to be used to scale up the system if necessary
			numberOfSame = 0;				//Reseting the numebrOfSame to '0'
			return 0;
		}
	}


	//To compare the user's input password if it is the same as the password stored
	int compare_password(char *password, char *x0F02F2E24C, char *x0F02F2DD3E)
	{
		if (strcmp(password, x0F02F2E24C) == 0)			//comparing the password of user 0F02F2E24C
		{
			return 1;									//returning 1 which will go into the switch case
		}

		else if (strcmp(password, x0F02F2DD3E) == 0)	//comparing the password of user 0F02F2DD3E
		{
			return 2;									//returning 2 which will go into the switch case
		}

		else
		{
			return 0;									//returning 0 which will go into the switch case
		}

	}


	//Database 'structure' declaration
	struct database
	{
		char name[40];						//declaring name of the person holding the unique ID
		int age;							//declaring age of the person holding the unique ID
		char password[10];					//declaring password of the person holding the unique ID
		int last_access_hour;				//declaring last access hour of the person holding the unique ID
		int last_access_minute;				//declaring last access minute of the person holding the unique ID
		int last_access_day;				//declaring last access day of the person holding the unique ID
		int last_access_month;				//declaring last access month of the person holding the unique ID
	};


	//function that prints the person's identity
	void printIdentity( struct database *fptr )
	{
		printf("Name: %s\n", (*fptr).name);																																//printing the person's name
		printf("Age: %d\n", (*fptr).age);																																	//printing the person's password
		printf("Password: %s\n", (*fptr).password);																														//printing the person's password
		printf("Last Access: %d:%d on Day:%d Month: %d\n", (*fptr).last_access_hour, (*fptr).last_access_minute, (*fptr).last_access_day, (*fptr).last_access_month);   //printing the person's last access time
	}


	//function that reads current time on the computer
	void getTime()
	{
		time_t now=time(NULL);				//read the current GMT time
		struct tm *tm=localtime(&now);		//reads the local time

		GMThour = tm->tm_hour;				//storing current GMT hour into the GMThour variable
		hour = GMThour + 8;					//we add '8' because Singapore's time is 8 hours faster than the GMT's time

		minute = tm->tm_min;				//storing current GMT minute into the minute variable

		day = tm->tm_mday;					//storing current GMT day into the day variable

		oldmonth = tm->tm_mon;				//storing current GMT month into the oldmonth variable
		month = oldmonth + 1;				//because the GMT month read starts from '0' - January, '1' - February  etc. Therefore, we need to add '1' to display the correct month to the user.

	}

	void sendbit_0000()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_ClearValue(0, 1<<4);
		GPIO_ClearValue(2, 1<<12);
	}

	void sendbit_0001()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_ClearValue(0, 1<<4);
		GPIO_SetValue(2, 1<<12);
	}

	void sendbit_0010()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_SetValue(0, 1<<4);
		GPIO_ClearValue(2, 1<<12);
	}

	void sendbit_0011()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_SetValue(0, 1<<4);
		GPIO_SetValue(2, 1<<12);
	}

	void sendbit_0100()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_SetValue(0, 1<<3);
		GPIO_ClearValue(0, 1<<4);
		GPIO_ClearValue(2, 1<<12);
	}

	void sendbit_0101()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_SetValue(0, 1<<3);
		GPIO_ClearValue(0, 1<<4);
		GPIO_SetValue(2, 1<<12);
	}

	void sendbit_0110()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_SetValue(0, 1<<3);
		GPIO_SetValue(0, 1<<4);
		GPIO_ClearValue(2, 1<<12);
	}

	void sendbit_0111()
	{
		GPIO_ClearValue(0, 1<<2);
		GPIO_SetValue(0, 1<<3);
		GPIO_SetValue(0, 1<<4);
		GPIO_SetValue(2, 1<<12);
	}

	void sendbit_1000()
	{
		GPIO_SetValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_ClearValue(0, 1<<4);
		GPIO_ClearValue(2, 1<<12);
	}

	void sendbit_1001()
	{
		GPIO_SetValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_ClearValue(0, 1<<4);
		GPIO_SetValue(2, 1<<12);
	}

	void sendbit_1010()
	{
		GPIO_SetValue(0, 1<<2);
		GPIO_ClearValue(0, 1<<3);
		GPIO_SetValue(0, 1<<4);
		GPIO_ClearValue(2, 1<<12);
	}


void RFID_scan(void)

{	int detect=TRUE;
	int currentState=0;
	int index_profession = 0;				//catching the return from compare_ID function
	int index_password = 0;					//catching the return from compare_password function

	int i = 0;								//declaring integer 'i' which will be passed into the compareID function
	int numberOfSame = 0;					//declaring 'numberOfSame' which will be passed into the compareID function
	int wrong_password = 0;					//declaring 'wrong_password' which we used as a counter

	struct database a0F02F2E24C = {"Larry", 22, "pass123", last_hour_a0F02F2E24C, last_minute_a0F02F2E24C, last_day_a0F02F2E24C, last_month_a0F02F2E24C};  //storing the data of one of the userID such as name, age, password, last access time
	struct database a0F02F2DD3E = {"Eric", 26, "123pass", last_hour_a0F02F2DD3E, last_hour_a0F02F2DD3E, last_hour_a0F02F2DD3E, last_hour_a0F02F2DD3E};	   //storing the data of one of the userID such as name, age, password, last access time

	init_uart2();							//calling the init_uart3 function we have initialised
	init_GPIO_2_13();						//calling the init_GPIO function we have initialized
	//init_GPIO_0_2();
	//init_GPIO_0_3();
	//init_GPIO_0_4();
	//init_GPIO_2_12();

	while(detect==TRUE){

	   start:								//the program will restart from here after it hits 'goto' below
		len = 0;							//initialising len as '0'
		GPIO_ClearValue(2, 1<<13);  		//To enable RFID reader
		printf("\n");

		//sendbit_0001();                     //scan card, sending 0001 to LCD screen
		currentState = compare_InputRFID(1,currentState);
		display_RFID(currentState);

		//RFID card reader reading data from card
		do{
			UART_Receive(LPC_UART2, &data, 1, BLOCKING); //function to receive data(byte) from RFID Reader

			if ((data != 10) && (data != 13))			//we need to read away the start bit(which is ASCII decimal 10 - new line) and the stop bit (which is ASCII decimal 13 - carriage return)
			{
			len++;										//if the data read is a 'newline' or carrige return, we dont store them into the array line[10]
			line[len-1] = data;
			}

		}while(len<10);  								//the unique ID only consists on 10 bytes. therefore, there is an array of 10 to store each of them. the do while loop will keep storing the data read until it hits the 10th byte data


		while(counter<2)								//since we our database only has '2' unique ID, we use counter less than '2'
		{
			if(strcmp(line, commandList[counter]) == 0)				//comparing the data read with the database we have in the commandlist
			{
				printf("Your accesss ID is '%s' !!\n", line);
				printf("Unlock, you are allowed to access!\n");
				//sendbit_0010();

				currentState = compare_InputRFID(2,currentState);
				display_RFID(currentState);//unlocked, sending 0010 to LCD screen

				Timer0_Wait(2000);
				index_profession = compareID(i, numberOfSame);		//passing 'i' and 'numberofSame' into the compareID function and catch the return value. We want to know the card holder's profession
				break;
			}

			else
		    {
				counter++;											//going through the commandlist until we found the matched ID
			}
		}

		switch (index_profession)									//depends on the return 'index_profession
		 {
			 case 1:
				 printf("You must be an Engineer\n");
				 sendbit_0101(); 									//engineer, sending 0101 to LCD screen
					currentState = compare_InputRFID(5,currentState);
					display_RFID(currentState);
				 Timer0_Wait(2000);
				 counter = 0;										//reseting counter
				 index_profession = 0;								//reseting index_profession
				 wrong_password = 0;								//reseting wrong_password
				 printf("Please Enter your password: \n");
				 //sendbit_0111(); 									//password, sending 0111 to LCD screen
					currentState = compare_InputRFID(7,currentState);
					display_RFID(currentState);
				 Timer0_Wait(3000);
				 scanf("%s", password);
			 break;

			 case 2:
				 printf("You must be a Designer\n");
				 //sendbit_0110(); 									//designer, sending 0110 to LCD screen
					currentState = compare_InputRFID(6,currentState);
					display_RFID(currentState);
				 counter = 0;										//reseting counter
				 index_profession = 0;								//reseting index_profession
				 wrong_password = 0;								//reseting wrong_password
				 printf("Please Enter your password: \n");			//prompting user to enter his/her password
				 //sendbit_0111(); 									//password, sending 0111 to LCD screen
					currentState = compare_InputRFID(7,currentState);
					display_RFID(currentState);
				 scanf("%s", password);
			 break;

			 default:
				 printf("Access Denied\n");
				 //sendbit_0011(); 									//denied, sending 0011 to LCD screen
					currentState = compare_InputRFID(3,currentState);
					display_RFID(currentState);
				 Timer0_Wait(2000);
				 printf("Sorry, your ID is in our database\n");
				 counter = 0;										//reseting counter
				 wrong_password = 0;								//reseting wrong_password
				 index_profession = 0;								//reseting index_profession
				 counter_denied++;									//counter to count how many times the person has tapped

				 if (counter_denied == 3)							//if the person has tapped 3 times it will break and go into the 'if' condition below
				 {
					 break;
				 }
				 else												//else, the program we re-run itself and go to the start again
				 {
				 goto start;
				 }
			 break;
		 }

		if(counter_denied == 3)										//if the person has tapped 3 times but access denied
		{
			//sendbit_0011(); 									    //denied, sending 0011 to LCD screen
			currentState = compare_InputRFID(3,currentState);
			display_RFID(currentState);
			Timer0_Wait(2000);
			//sendbit_0100(); 										//warning, sending 0100 to LCD screen
			currentState = compare_InputRFID(4,currentState);
			display_RFID(currentState);
			GPIO_SetValue(2, 1<<13);								//disable the RFID card reader
			Timer0_Wait(5000);										//wait for 5 seconds
			counter_denied = 0;										//clear the counter_denied
			goto start;												//go to the start of the program
		}

		//passing the input password from the user and the database passwords into this function
		index_password = compare_password(password, a0F02F2E24C.password, a0F02F2DD3E.password);		//if access is granted, it wont go into the default of the switch case, above but into this function

		restart2:

			if(wrong_password == 2)																//the user key in the wrong password for the next two times
			{
				//sendbit_0011(); 																//denied, sending 0011 to LCD screen
				currentState = compare_InputRFID(3,currentState);
				display_RFID(currentState);
				Timer0_Wait(2000);
				//sendbit_0100(); 																//warning, sending 0100 to LCD screen
				currentState = compare_InputRFID(4,currentState);
				display_RFID(currentState);
				GPIO_SetValue(2, 1<<13);														//disable the RFID card reader
				Timer0_Wait(5000);																//time delay for 5 seconds
				wrong_password = 0;																//reseting the wrong_password
				//goto start;																		//restarting the program
				detect=FALSE;
			}

		restart1:

		switch(index_password)																	//the return value from the compare_password function will go into this switch case
		{
			case 1:
				printf("Welcome %s\n", a0F02F2E24C.name);
				getTime();																		//getTime function to read the current system time
				a0F02F2E24C.last_access_hour = last_hour_a0F02F2E24C;							//to put value of last access hour, minute, day, month of a0F02F2E24C into the structure
				a0F02F2E24C.last_access_minute = last_minute_a0F02F2E24C;
				a0F02F2E24C.last_access_day = last_day_a0F02F2E24C;
				a0F02F2E24C.last_access_month = last_month_a0F02F2E24C;
				//sendbit_1010(); 																//access, sending 1010 to LCD screen
				currentState = compare_InputRFID(10,currentState);
				display_RFID(currentState);
				Timer0_Wait(2000);
				printIdentity(&a0F02F2E24C);													//to print the identity of the unique ID user

				last_hour_a0F02F2E24C = hour;													//assigning the current read hour, minute, day, month to last access hour, minute, day, month
				last_minute_a0F02F2E24C = minute;
				last_day_a0F02F2E24C = day;
				last_month_a0F02F2E24C = month;

				index_password = 0;																//reseting index_password to '0'
				detect=FALSE;
				break;

			case 2:
				printf("Welcome %s\n", &a0F02F2DD3E.name);
				getTime();																		//getTime function to read the current system time
				a0F02F2DD3E.last_access_hour = last_hour_a0F02F2DD3E;							//to put value of last access hour, minute, day, month of a0F02F2E24C into the structure
				a0F02F2DD3E.last_access_minute = last_minute_a0F02F2DD3E;
				a0F02F2DD3E.last_access_day = last_day_a0F02F2DD3E;
				a0F02F2DD3E.last_access_month = last_month_a0F02F2DD3E;
				//sendbit_1010(); 																//access, sending 1010 to LCD screen
				currentState = compare_InputRFID(10,currentState);
				display_RFID(currentState);
				Timer0_Wait(2000);
				printIdentity(&a0F02F2DD3E);													//to print the identity of the unique ID user

				last_hour_a0F02F2DD3E = hour;													//assigning the current read hour, minute, day, month to last access hour, minute, day, month
				last_minute_a0F02F2DD3E = minute;
				last_day_a0F02F2DD3E = day;
				last_month_a0F02F2DD3E = month;

				index_password = 0;																//reseting index_password to '0'
				detect=FALSE;
				break;

			default:

				if(wrong_password == 1)															//to print out the correct line depends how many times the user enter his/her password wrongly
				{
					printf("You are only allowed to enter for one more trials\n");
					//sendbit_1000(); 															//reenter_pass1, sending 1000 to LCD screen
					currentState = compare_InputRFID(8,currentState);
					display_RFID(currentState);
				}

				else
				{
				printf("You are only allowed to enter for two more trials\n");
				printf("Thereafter, the card reader will be locked for 5 seconds\n");

				//sendbit_1001(); 																//reenter_pass2, sending 1001 to LCD screen
				currentState = compare_InputRFID(9,currentState);
				display_RFID(currentState);
				Timer0_Wait(2000);
				}

				printf("Please reenter your password: \n");
				scanf("%s", password);
				wrong_password++;																				//counter to count how many times password has been wrongly entered
				index_password = compare_password(password, a0F02F2E24C.password, a0F02F2DD3E.password);		//to pass the new password entered and re-comparing them

				if(index_password != 0){																		//if the index_password returned is not '0'
					goto restart1;																				//the program will start from restart1
					wrong_password = 0;																			//reseting wrong_password counter
				}

				else
				{
					goto restart2;																				//if the index_password returned is '0', the program will start to run again at restart2
				}

			break;
		}


/*
	char expected[10] = "0F02F2DD3E";
	for(i=0;i<10;i++)
	printf("expecting %c %d,%c\n",expected[i], line[i], line[i]);

		printf("expecting 0 %c\n", line[0]);
		printf("expecting F %c\n", line[1]);
		printf("expecting 0 %c\n", line[2]);
		printf("expecting 2 %c\n", line[3]);
		printf("expecting F %c\n", line[4]);
		printf("expecting 2 %c\n", line[5]);
		printf("expecting D %c\n", line[6]);
		printf("expecting D %c\n", line[7]);
		printf("expec5ting 3 %c\n", line[8]);
		printf("expecting E %c\n", line[9]);


*/

	}
}


void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}

