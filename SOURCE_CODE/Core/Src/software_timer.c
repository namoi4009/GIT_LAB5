/*
 * software_timer.c
 *
 *  Created on: Dec 4, 2023
 *      Author: tango
 */

#include "software_timer.h"

int timer_counter[NO_OF_TIMER];
int timer_flag[NO_OF_TIMER];

void setTimer(int duration, int ID){
	timer_counter[ID] = duration/TICK;
	timer_flag[ID] = 0;
}

void timerRun(){
	for(int i = 0; i<NO_OF_TIMER; i++){
		if(timer_counter[i] > 0){
			timer_counter[i]--;
			if(timer_counter[i] <= 0){
				timer_flag[i] = 1;
			}
		}
	}
}

void initTimer(int delay){
	for(int i = 0; i<NO_OF_TIMER; i++){
		setTimer(delay, i);
	}
}
