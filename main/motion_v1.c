
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "LPC17xx.h"
#include "stdio.h"
#include "lpc17xx_timer.h"
#include "lcd.h"

#define x 2 //port 2
#define y 0 //port 0
#define READY 3




static int scanning=FALSE;
static int oldx = -1;	//previous values, constantly updated when there is a continuous gesture,
static int oldy = -1;  	//reset after a short idle
static int command=IDLE;

static int leftCount =  0;
static int rightCount = 0;
static int upCount = 0;
static int downCount = 0;



uint32_t pin(int port, uint32_t input_pin_no){

	 return ((GPIO_ReadValue(port)>> input_pin_no) % 2);

}

void init_motion(){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 1;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;

	//initialize x @ Port 2
	int count = 0;
	while (count <8){ //P2.0 to P2.7
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

	count = 19;
	PinCfg.Portnum = 0;
	while (count <23){	//P0.19 to P0.22
	PinCfg.Pinnum = count;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(0, (1<<count),1); //setting it as output
	count++;
	}


}

/*void sensor_debug(){

	int count;

	while(1){
		GPIO_SetValue(0,1<<2);

		printf("X ");

		count=0;
		while(count<8){
		printf("%x", pin(2,count));
		count++;
		}

		printf("   Y ");

		count=15;
		while(count<19){
		printf("%x", pin(0,count));
		count++;
		}
		count=23;
		while(count<27){
		printf("%x", pin(0,count));
		count++;
		}

		printf("\n");

	}

}

*/
int scan(int axis, int start, int end){
	int count=start;
	int repeat = 0;
	scanning=TRUE;
	//printf("oldx is %d\n", oldx);
	while(count<=end && scanning==TRUE){
		if(pin(axis, count)==1){
			compare(axis, count);
		}
		count++;
		if(count==end+1){
			count=start;
			repeat++;
			//printf("repeat %d\n", repeat);
		}
		if(repeat >1000){ //allow 50000 cycles of idle mode
			oldx=0;
			oldy=0; //reset to zero
			scanning=FALSE;
			command=IDLE;
			reset_valid();

			//printf("reset\n");
		}

	}
	//printf("repeat is %d\n", repeat);
	return command;
}

void compare(int axis, int coordinate){
	int difference=0;

		if(old(axis)!=-1){
			difference = coordinate - old(axis);

			direction(axis, difference);

		}
		if(old(axis)==-1){
			//do nothing
		}

		old_update(axis, coordinate);
		scanning = FALSE; //exit scan loop, from x to y or y to x

}

int old(int axis){
	if(axis==x){
		return oldx;
	}
	if(axis==y){
		return oldy;
	}
}

void old_update(int axis, int coordinate){
	if(axis==x){
		oldx=coordinate;
		//printf("coordinate x is %d\n", oldx);
	}
	if(axis==y){
		oldy=coordinate;
		//printf("coordinate y is %d\n", oldy);
	}
}

void direction(int axis, int difference){
	if(difference>0){
		if(axis==x){
			rightCount++;
			leftCount-=1;
			//printf("right is %d\n",rightCount);
			if (rightCount>=2)
			{
			right();
			}
		}
		if(axis==y){
			upCount++;
			downCount-=1;
			//printf("up is %d\n",upCount);
			if (upCount>=2)
			{
				up();
			}
		}

	}

	if(difference<0){
		if(axis==x){
			leftCount++;
			rightCount-=1;
			//printf("left is %d\n",leftCount);
			if (leftCount>=2)
			{
			left();
			}
		}
		if(axis==y){
			downCount++;
			upCount-=1;
			//printf("down is %d\n",downCount);
			if (downCount>=2)
			{
			down();
			}
		}

	}

}

//****directional commands***********
//printf is slow, try to light up 4 LEDs instead
void up(){
	//GPIO_SetValue(0, 1<<UP);
	//Timer0_Wait(500);
	reset_valid();
	command=UP;
	printf("up\n");
}

void down(){
	//GPIO_SetValue(0, 1<<DOWN);
	//Timer0_Wait(500);
	reset_valid();
	command=DOWN;
	printf("down\n");
}

void left(){
	//GPIO_SetValue(0, 1<<LEFT);
	//Timer0_Wait(500);
	reset_valid();
	command=LEFT;
	printf("left\n");
}

void right(){
	//GPIO_SetValue(0, 1<<RIGHT);
	//Timer0_Wait(500);
	reset_valid();
	command=RIGHT;
	printf("right\n");
}

void reset_valid(void)
{
	leftCount =  0;
	rightCount = 0;
	upCount = 0;
	downCount = 0;
}

int select(){
	int pin_count=0;
	pin_count=multiple_scan(); //code will stuck here
	if(pin_count>3){
		command=SELECT;
		return command;
	}
	else command=IDLE;
		return command;
}

int multiple_scan(){

	int count=0;
	int position=0;
	int repeat=0;
	int temp1,temp2,temp3;
	int loop=0;
	int pin_count;

	while(count<=8){
		if(pin(2, count)==1 && position==0){
			temp1=count; //locked to this value
			position++;
			//printf("temp1 is %d\n", temp1);
		}

		if(position==1 && repeat<1000){
			if(pin(2, count)==1 && count!= temp1){//make sure that they are different coordinates
			temp2=count;
			//printf("temp2 is %d\n", temp2);
			position++;
			}
		}

		if(position==2 && repeat<1000){
			if(pin(2, count)==1 && count!= temp1 && count!=temp2){//make sure that they are different coordinates
			temp3=count;
			//printf("temp2 is %d\n", temp2);
			position=READY;
			}
		}
		count++;
		if(count==8){
			count=0;

			repeat++;

		}

		while(position==READY && loop<10000){
			while(pin(2, temp1)==1 && pin(2, temp2)==1 && pin(2, temp3)==1 && loop<=9999){
							loop++; //if both were locked
							//printf("loop is %d\n", loop);
						}
			if(loop==10000){
				pin_count=4;
				return pin_count;
			}
			if(loop<9999){
				pin_count=0;

				return pin_count;
			}
		}

		if(repeat>1000){
			if(position>0){
				pin_count=0;
				return pin_count;
			}
//			printf("reset\n");
			repeat=0;
			loop=0;
			position=0;
			count=0; //reset
		}


	}
}

int fast_scan(int axis, int start, int end){
	int count=start;
	int repeat = 0;
	scanning=TRUE;
	while(count<end && scanning==TRUE){
		if(pin(axis, count)==1){
			fast_compare(axis, count);
		}
		if(count==end){
			count=start-1;
			repeat++;
			//printf("repeat %d\n", repeat);
		}
		if(repeat >50000){ //allow 50000 cycles of idle mode
			oldx=0;
			oldy=0; //reset to zero
			scanning=FALSE;
			//printf("reset\n");
		}
		count++;
	}
	Timer0_Wait(500);
	return command;
}

void fast_compare(int axis, int coordinate){
	int difference=0;

		if(old(axis)!=-1){
			difference = coordinate - old(axis);

			fast_direction(axis, difference);

		}
		if(old(axis)==-1){
			//do nothing
		}

		old_update(axis, coordinate);
		scanning = FALSE; //exit scan loop, from x to y or y to x

}

void fast_direction(int axis, int difference){
	if(difference>0){
		if(axis==x){
			right();
		}
		if(axis==y){
			up();
		}
	}

	if(difference<0){
		if(axis==x){
			left();
		}
		if(axis==y){
			down();
		}
	}

}

int ultra_fast_scan(){
	int count;
	count=15;
	while(1){
	if(pin(0, count)==1){
		command= count;
		return command;
	}
	count++;
	if(count>18){
		count=15;
	}
	}
}
