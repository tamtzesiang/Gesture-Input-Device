
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

#define SINGLE 1
#define MULTI 2
#define UP_LEFT		5
#define UP_RIGHT	7
#define DOWN_LEFT	6
#define DOWN_RIGHT	8

int static cmd_up=FALSE;
int static cmd_down=FALSE;
int static cmd_left=FALSE;
int static cmd_right=FALSE;

int static cmd_zoom_in=FALSE;
int static cmd_zoom_out=FALSE;

int static oldx[8];
int static oldy[8];
int static newx[8];
int static newy[8];

int static tempx[8];
int static tempy[8];

int static selecting=FALSE;
int static cmd_select=FALSE;

int static x_difference=0;
int static y_difference=0;

int static difference=0;

void sensor_debug(){
	int count=0;

	while(1){

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



void scan_and_store(){
	int x=2;
	int y=0;
	int count=0;
	while(count<8){
		if(pin(x,count)==1){
			tempx[count]=1;
		}
		if(pin(x,count)==0){
			tempx[count]=0;
		}
		count++;
	}
	count=15;
	while(count<19){
		if(pin(y,count)==1){
			tempy[count-15]=1;
		}
		if(pin(y,count)==0){
			tempy[count-15]=0;
		}
		count++;
	}
	count=23;
	while(count<27){
		if(pin(y,count)==1){
			tempy[count-19]=1;
		}
		if(pin(y,count)==0){
			tempy[count-19]=0;
		}
		count++;
	}
	//print_array(tempx);
	//print_array(tempy);

}

int input_count(int array[8]){
/*	printf("oldx is ");
	print_array(oldx);
	printf("oldy is ");
	print_array(oldy);
*/
	int count=0;
	int input=0;
	while(count<8){
		if(array[count]==1){
			input++;
		}
		count++;
	}
	return input;
}

void get_oldx(){
	int count=0;
	while(count<8){
		oldx[count]=tempx[count];
		count++;
	}
}

void get_oldy(){
	int count=0;
	while(count<8){
		oldy[count]=tempy[count];
		count++;
	}
}

int any_touch(){
	scan_and_store();
	if(input_count(tempx)>0 && input_count(tempy)>0){
		return TRUE;
	}
	return FALSE;
}

void get_newx(){
	int count=0;
	while(count<8){
		newx[count]=tempx[count];
		count++;
	}
}

void get_newy(){
	int count=0;
	while(count<8){
		newy[count]=tempy[count];
		count++;
	}
}

int get_gap(int array[8]){ //returns 1 to 7
	int first=0;
	int last=0;
	int count=0;
	int gap=-1;
	while(count<8){
		if(array[count]==1 && first==0){
			first=count+1;
		}
		if(array[count]==1 && first!=0){ //if [0][1][1][1][1][1][0][0], bit 0 and 5 will be considered
			last=count+1;
		}
		count++;
	}
	gap=last-first;

	return gap;
}

int touch_check(){
	int check=0;
	int repeat=0;
	while(input_count(tempx)>=0 && input_count(tempy)>=0 && repeat<10000){
		scan_and_store();
	if(input_count(tempx)>0 && input_count(tempy)>0 && get_gap(tempx)<2 && get_gap(tempy)<2){
		get_newx();
		get_newy();
		//scan_and_store();
		check=SINGLE;
		repeat=0;
	}
	else repeat++;
	}
	if((input_count(tempx)>1 || input_count(tempy)>1) && (get_gap(tempx)>1 ||get_gap(tempy)>1)){
		scan_and_store();
		get_oldx();
		get_oldy();
		check=MULTI;
	}
	//if(repeat==1000){
		//printf("repeat done\n");
	//}
	return check;
}

void compare_single(){
	int point1x=-1;
	int point1y=-1;
	int point2x=-1;
	int point2y=-1;

	if(input_count(oldx)==2){
		point1x=average(oldx);
	}
	if(input_count(oldx)==1){
		point1x=get_point(oldx);
	}
	if(input_count(newx)==2){
		point2x=average(newx);
	}
	if(input_count(newx)==1){
		point2x=get_point(newx);
	}

	if(input_count(oldy)==2){
	point1y=average(oldy);
	}
	if(input_count(oldy)==1){
		point1y=get_point(oldy);
	}
	if(input_count(newy)==2){
		point2y=average(newy);
	}
	if(input_count(newy)==1){
		point2y=get_point(newy);
	}

//printf("point2x %d ", point2x);
//printf("point1x %d\n", point1x);
//printf("point2y %d ", point2y);
//printf("point1y %d\n", point1y);

	if(point2x-point1x > 0){
		right1();
		x_difference=point2x-point1x;
	}
	if(point2x-point1x < 0){
		left1();
		x_difference=point2x-point1x;
	}
	if(point2y-point1y <0){
		up1();
		y_difference=point2y-point1y;
	}
	if(point2y-point1y > 0){
		down1();
		y_difference=point2y-point1y;
	}

}

int average(int array[8]){
	int count=0;
	int point1=-1;
	int point2=-1;
	while(count<8){
		if(array[count]==1){
			point1=count;
		}
		if(array[count]==1 && point1!=-1){
			point2=count;
		}
		count++;
	}
	return (point1+point2)/2;
}

int get_point(int array[8]){
	int count=0;
	while(count<8){
		if(array[count]==1){
			return count;
		}
		count++;
	}
}

void get_multi(){
	int repeat=0;
	while(input_count(tempx)>=0 && input_count(tempy)>=0 && repeat<20000){
		scan_and_store();
		if((input_count(tempx)>1 || input_count(tempy)>1) && (get_gap(tempx)>1 ||get_gap(tempy)>1)){
			get_newx();
			get_newy();
		}
		else repeat++;

	}
	if(input_count(oldx)>=5 || input_count(oldy)>=5){
		selecting=TRUE;
		cmd_select=TRUE;
	}
}

void compare_multi(){
	if(get_gap(newx)-get_gap(oldx)>1){
		//printf("oldx");
		//print_array(oldx);
		//printf("newx");
		//print_array(newx);
		//printf("x ");
		zoom_in();
		x_difference=get_gap(newx)-get_gap(oldx);
		x_difference=get_difference_x();
	}
	if(get_gap(newx)-get_gap(oldx)<-1){
		//printf("oldx");
		//print_array(oldx);
		//printf("newx");
		//print_array(newx);
		//printf("x ");
		zoom_out();
		x_difference=get_gap(newx)-get_gap(oldx);
		x_difference=get_difference_x();
	}
	if(get_gap(newy)-get_gap(oldy)>1){
		//printf("oldy");
		//print_array(oldy);
		//printf("newy");
		//print_array(newy);
		//printf("y ");
		zoom_in();
		y_difference=get_gap(newy)-get_gap(oldy);
		y_difference=get_difference_y();
	}
	if(get_gap(newy)-get_gap(oldy)<-1){
		//printf("oldy");
		//print_array(oldy);
		//printf("newy");
		//print_array(newy);
		//printf("y ");
		zoom_out();
		y_difference=get_gap(newy)-get_gap(oldy);
		y_difference=get_difference_y();
	}
	if(y_difference>x_difference){
		difference=y_difference;
	}
	if(y_difference<x_difference){
		difference=x_difference;
	}
}

void reset(){
	int count=0;
	while(count<8){
		oldx[count]=0;
		newx[count]=0;
		tempx[count]=0;

		oldy[count]=0;
		newy[count]=0;
		tempy[count]=0;
		count++;
	}

}

//commands
void up1(){
	//printf("up1\n");
	cmd_up=TRUE;
}

void down1(){
	//printf("down1\n");
	cmd_down=TRUE;
}

void left1(){
	//printf("left1\n");
	cmd_left=TRUE;
}

void right1(){
	//printf("right1\n");
	cmd_right=TRUE;
}

void zoom_in(){
	//printf("zoom_in\n");
	cmd_zoom_in=TRUE;
}

void zoom_out(){
	//printf("zoom_out\n");
	cmd_zoom_out=TRUE;
}



int return_command_single(){

	int change_to=0;
	if(cmd_up==TRUE && cmd_left==TRUE){
		if(get_difference_y()>get_difference_x()){
			difference=get_difference_y()-get_difference_x();
			change_to=UP;
		}
		if (get_difference_y()<get_difference_x()){
			difference=get_difference_x()-get_difference_y();
			change_to=LEFT;
		}
		if(difference<2){
		return UP_LEFT;
		}
		else return change_to;
	}
	if(cmd_up==TRUE && cmd_right==TRUE){
		if(get_difference_y()>get_difference_x()){
			difference=get_difference_y()-get_difference_x();
			change_to=UP;
		}
		if (get_difference_y()<get_difference_x()){
			difference=get_difference_x()-get_difference_y();
			change_to=RIGHT;
		}
		if(difference<2){
		return UP_RIGHT;
		}
		else return change_to;
	}
	if(cmd_down==TRUE && cmd_left==TRUE){
		if(get_difference_y()>get_difference_x()){
			difference=get_difference_y()-get_difference_x();
			change_to=DOWN;
		}
		if (get_difference_y()<get_difference_x()){
			difference=get_difference_x()-get_difference_y();
			change_to=LEFT;
		}
		if(difference<2){
		return DOWN_LEFT;
		}
		else return change_to;
	}
	if(cmd_down==TRUE && cmd_right==TRUE){
		if(get_difference_y()>get_difference_x()){
			difference=get_difference_y()-get_difference_x();
			change_to=DOWN;
		}
		if (get_difference_y()<get_difference_x()){
			difference=get_difference_x()-get_difference_y();
			change_to=RIGHT;
		}
		if(difference<2){
		return DOWN_RIGHT;
		}
		else return change_to;
	}
	if(cmd_up==TRUE){
		difference=get_difference_y();
		return UP;
	}
	if(cmd_down==TRUE){
		difference=get_difference_y();
		return DOWN;
	}
	if(cmd_left==TRUE){
		difference=get_difference_x();
		return LEFT;
	}
	if(cmd_right==TRUE){
		difference=get_difference_x();
		//printf("returned RIGHT\n");
		return RIGHT;
	}
	else return 0;
}

int return_command_multi(){
	if(cmd_zoom_in==TRUE){
		return ZOOM_IN;
	}
	if(cmd_zoom_out==TRUE){
		return ZOOM_OUT;
	}
	if(cmd_select==TRUE){
		return SELECT;
	}
}

void clear_command(){
	cmd_up=FALSE;
	cmd_down=FALSE;
	cmd_left=FALSE;
	cmd_right=FALSE;
	cmd_zoom_in=FALSE;
	cmd_zoom_out=FALSE;
	cmd_select=FALSE;
	selecting=FALSE;
}

int get_command(){
	int check=0;
	int commands=0;

	difference=0;
	x_difference=0;
	y_difference=0;
	clear_command();
	reset();

	scan_and_store();
	get_oldx();
	get_oldy();

	//printf("input count for old x is %d\n", input_count(oldx));
	//printf("input count for old y is %d\n", input_count(oldy));
	//printf("gap for old x is %d\n", get_gap(oldx));
	//printf("gap for old y is %d\n", get_gap(oldy));

	if(input_count(oldx)>=1 && input_count(oldy)>=1 && get_gap(oldx)<2 && get_gap(oldy)<2){
		check=touch_check();
		if(check==SINGLE){
			//get_newx();
			//get_newy();
				//printf("newx is ");
				//print_array(newx);
				//printf("newy is ");
				//print_array(newy);

			compare_single();
			commands=return_command_single();
			//printf("commands in get command is %d\n", commands);
		}
		if(check==MULTI){
			get_multi();
			if(selecting==FALSE){
			compare_multi();
			commands=return_command_multi();
			}
		}

	}
	if(input_count(oldx)>1 && input_count(oldy)>1 && get_gap(oldx)>1 && get_gap(oldy)>1) {
		get_multi();
		if(selecting==FALSE){
		compare_multi();
		}
		commands=return_command_multi();
	}

	if(input_count==5){
		commands=SELECT;
	}

	return commands;
}

/*void print_array(int array[8]){

	int count=0;
	while(count<8){
		printf("%d", array[count]);
		count++;
	}
	printf("\n");
}
*/

int get_difference_x(){
	if(x_difference<0){
		x_difference=x_difference*-1;
	}
	return x_difference;
}

int get_difference_y(){
	if(y_difference<0){
		y_difference=y_difference*-1;
	}
	return y_difference;
}

int get_difference(){
	return difference;
}
