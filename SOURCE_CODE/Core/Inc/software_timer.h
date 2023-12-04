/*
 * software_timer.h
 *
 *  Created on: Dec 4, 2023
 *      Author: tango
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#define NO_OF_TIMER	3
#define TICK 10

// Function declarations
extern int timer_flag[NO_OF_TIMER];
void setTimer(int duration, int ID);
void timerRun();
void initTimer(int delay);

#endif /* INC_SOFTWARE_TIMER_H_ */
