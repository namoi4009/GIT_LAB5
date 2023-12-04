/*
 * scheduler.c
 *
 *  Created on: Dec 4, 2023
 *      Author: tango
 */

#include "scheduler.h"

typedef struct {
	void ( * pTask)(void);
	uint32_t Delay;
	uint32_t Period;
	uint8_t RunMe;
	uint32_t TaskID;
} sTask;

// The array of tasks
static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint32_t newTaskID = 0;
static uint32_t Error_code_G = 0;

static uint32_t Get_New_Task_ID(void){
	newTaskID++;
	if(newTaskID == NO_TASK_ID){
		newTaskID++;
	}
	return newTaskID;
}

void SCH_Go_To_Sleep(){
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void Timer_init(){
	for (int i = 0; i < 5; i++)
	{
		setTimer(0, i);
	}
}

void SCH_Init(void){
	Error_code_G = 0;
	Watchdog_init();
	Timer_init();
}

void SCH_Update(void){
	// Check if there is a task at this location
	if (SCH_tasks_G[0].pTask && SCH_tasks_G[0].RunMe == 0) {
		if(SCH_tasks_G[0].Delay > 0){
			SCH_tasks_G[0].Delay = SCH_tasks_G[0].Delay - 1;
		}
		if (SCH_tasks_G[0].Delay == 0) {
			SCH_tasks_G[0].RunMe = 1;
		}
	}
}
uint32_t SCH_Add_Task(void (* pFunction)(), uint32_t DELAY, uint32_t PERIOD){
	uint8_t newTaskIndex = 0; // Index for iterating through tasks
	uint32_t sumDelay = 0; // Accumulator for total delay time
	uint32_t newDelay = 0; // Delay time for the new task

	// Iterate through tasks to find a suitable position for the new task
	for(newTaskIndex = 0; newTaskIndex < SCH_MAX_TASKS; newTaskIndex ++){
		sumDelay += SCH_tasks_G[newTaskIndex].Delay;

		// Check if the current position is suitable for the new task
		if(sumDelay > DELAY)
		{
			// Calculate the delay for the new task and adjust the delay of the current task
			newDelay = DELAY - (sumDelay - SCH_tasks_G[newTaskIndex].Delay);
			SCH_tasks_G[newTaskIndex].Delay = sumDelay - DELAY;

			// Shift tasks down to make room for the new task
			for(uint8_t i = SCH_MAX_TASKS - 1; i > newTaskIndex; i--)
			{
				SCH_tasks_G[i] = SCH_tasks_G[i - 1];
			}

			// Insert the new task at the found position
			SCH_tasks_G[newTaskIndex].pTask = pFunction;
			SCH_tasks_G[newTaskIndex].Delay = newDelay;
			SCH_tasks_G[newTaskIndex].Period = PERIOD;
			SCH_tasks_G[newTaskIndex].RunMe = (newDelay == 0) ? 1 : 0;
			SCH_tasks_G[newTaskIndex].TaskID = Get_New_Task_ID();
			return SCH_tasks_G[newTaskIndex].TaskID;
		} else if(SCH_tasks_G[newTaskIndex].pTask == NULL)
		{
			// If an empty slot is found, insert the new task
			SCH_tasks_G[newTaskIndex].pTask = pFunction;
			SCH_tasks_G[newTaskIndex].Delay = DELAY - sumDelay;
			SCH_tasks_G[newTaskIndex].Period = PERIOD;
			SCH_tasks_G[newTaskIndex].RunMe = (SCH_tasks_G[newTaskIndex].Delay == 0) ? 1 : 0;
			SCH_tasks_G[newTaskIndex].TaskID = Get_New_Task_ID();
			return SCH_tasks_G[newTaskIndex].TaskID;
		}
	}

	// Return a special value (such as 0 or an error code) if no space is available
	return NO_TASK_ID; // Example error code for no space
}



uint8_t SCH_Delete_Task(uint32_t taskID){
	uint8_t Return_code  = 0;
	uint8_t taskIndex;
	uint8_t j;
	if(taskID != NO_TASK_ID){
		for(taskIndex = 0; taskIndex < SCH_MAX_TASKS; taskIndex ++){
			if(SCH_tasks_G[taskIndex].TaskID == taskID){
				Return_code = 1;
				if(taskIndex != 0 && taskIndex < SCH_MAX_TASKS - 1){
					if(SCH_tasks_G[taskIndex+1].pTask != 0x0000){
						SCH_tasks_G[taskIndex+1].Delay += SCH_tasks_G[taskIndex].Delay;
					}
				}

				for( j = taskIndex; j < SCH_MAX_TASKS - 1; j ++){
					SCH_tasks_G[j].pTask = SCH_tasks_G[j+1].pTask;
					SCH_tasks_G[j].Period = SCH_tasks_G[j+1].Period;
					SCH_tasks_G[j].Delay = SCH_tasks_G[j+1].Delay;
					SCH_tasks_G[j].RunMe = SCH_tasks_G[j+1].RunMe;
					SCH_tasks_G[j].TaskID = SCH_tasks_G[j+1].TaskID;
				}
				SCH_tasks_G[j].pTask = 0;
				SCH_tasks_G[j].Period = 0;
				SCH_tasks_G[j].Delay = 0;
				SCH_tasks_G[j].RunMe = 0;
				SCH_tasks_G[j].TaskID = 0;
				return Return_code;
			}
		}
	}
	return Return_code; // return status
}

void SCH_Dispatch_Tasks(void){
	if(SCH_tasks_G[0].RunMe > 0) {
		(*SCH_tasks_G[0].pTask)(); // Run the task
		SCH_tasks_G[0].RunMe = 0; // Reset flag
		sTask temtask = SCH_tasks_G[0];
		SCH_Delete_Task(temtask.TaskID);
		if (temtask.Period != 0) {
			SCH_Add_Task(temtask.pTask, temtask.Period, temtask.Period);
		}
	}
	SCH_Go_To_Sleep();
}
