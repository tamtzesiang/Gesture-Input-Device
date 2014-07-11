#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "LPC17xx.h"
#include "stdio.h"
#include "lpc17xx_timer.h"
#include "LCD.h"
#include "motion.h"
//#include "multi_touch.h"

#define x 2 //port 2
#define y 0 //port 0

static int arrayx[6]={0};
static int array_oldx[6]={0};
static int array_newx[6]={0};

static int cmd=IDLE;

/*
void init_GPIO(){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 1;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;

	//initialize x @ Port 2
	int count = 0;
	while (count <6){ //P2.0 to P2.5
	PinCfg.Pinnum = count;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(2, (1<<count),0); //setting it as input
	count++;
	}

	//initialize y @ Port 0
	count = 15;
	PinCfg.Portnum = 0;
	while (count <19){	//P0.15 to P0.18
	PinCfg.Pinnum = count;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(0, (1<<count),0); //setting it as input
	count++;
	}

	//initialize y @ Port 0
	count = 23;
	PinCfg.Portnum = 0;
	while (count <27){	//P0.23 to P0.26
	PinCfg.Pinnum = count;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(0, (1<<count),0); //setting it as input
	count++;
	}

}
*/

//Function to check whether there are 2 inputs and straight away capture the 2 coordinates in arrayx
int check_multitouch(){
	int count=0;
	int count_one=0; //number of "one"
	int valid=FALSE;

	for(count=0; count<6; count++){
		arrayx[count]=pin(2, count); //storing the initial points
		if(pin(2,count)==1){
			count_one++;
		}
		if(count_one==2){
			valid=TRUE;
		}
	}
	return valid;
}

//Function to check whether there is any input, return TRUE if there is, otherwise FALSE
int check_anytouch(){
	int count=0;
	for(count=0; count<6; count++){
		if(pin(2,count)==1){
			return TRUE;
		}
	}
	return FALSE;
}

//Function to store the 2 initial coordinates
void capture_oldx(){
	int count=0;
	//printf("OLD : ");
	for(count=0; count<6; count++){
		array_oldx[count]=arrayx[count];
		//printf("%d",array_oldx[count]);
	}
	//printf("\n");
}

//Function to store the 2 latest coordinates
void capture_newx(){
	int count=0;
	//printf("NEW : ");
	while(check_anytouch()==TRUE){				 //to prevent it from misunderstand the user has lift up his fingers
		if(check_multitouch()==TRUE){			 //one finger will still in while loop but wont update the array
			for(count=0; count<6; count++){		 //array_newx will only be updated if there are multitouch
				array_newx[count]=arrayx[count]; //constantly update the 2 new coordinates until he lift his fingers (transfer from arrayx)
			}
		}
	}
	/*for(count=0; count<6; count++){		//for debugging
		printf("%d",array_newx[count]);
	}
	printf("\n");*/
}

//Function to compare initial and latest 2 points (analyse which command it is)
int compare_x(int oldx[6], int newx[6]){
	int count=0;
	int old_distance=0;
	int new_distance=0;
	int distance=0;

	printf("OLD : ");
	for(count=0; count<6; count++){		//for debugging
		printf("%d",oldx[count]);
	}
	printf("\n");

	printf("NEW : ");
	for(count=0; count<6; count++){		//for debugging
		printf("%d",newx[count]);
	}
	printf("\n");

	old_distance = calculate_dis(oldx);
	new_distance = calculate_dis(newx);

	distance = new_distance-old_distance;

	//printf("distance = %d \n",distance);

	if(distance>0){				//positive meaning zoom in
		//printf("zoom in\n");
		cmd=ZOOM_IN;
	}
	else if(distance<0){		//negative meaning zoom out
		//printf("zoom out\n");
		cmd=ZOOM_OUT;
	}
	return distance;
}

//Function to calculate the distance between 2 points
int calculate_dis(int array[6]){
	int distance=0;
	int count=0;
	int x1=-1;
	int x2=-1;

	for(count=0; count<6; count++){
		if(array[count]==1 && x1==(-1)){
			x1=count;
			array[count]=0; //clear the 1st "one" so that the next "one" will be store in x2
		}
	}
	for(count=0; count<6; count++){
		if(array[count]==1 && x2==(-1)){
			x2=count;
		}
	}
	distance = x2-x1;
	return distance;
}


/******************************************************************************
 **   Main Function  main()
 ******************************************************************************/
int multitouch(){

	cmd=IDLE; //reset it each time it's being called


	//init_GPIO();
	int valid=FALSE;
	int distance=0;

	while(cmd==IDLE){//NOTE: LOOP WILL EXIT UNTIL A COMMAND IS DETECTED!!!!!
		valid=check_multitouch();			//check whether got 2 points
		//printf("validity = %d\n",valid);	//for debugging
		if (valid==TRUE){
			capture_oldx();					//if it is valid, store the 2 X coordinates where the user inputs
			capture_newx();					//capture the 2 new X coordinates before the user lift his fingers
			distance = compare_x(array_oldx,array_newx);
		}
	}
	return distance;
}

int command(){
	return cmd;
}
