/*
 * Scheduler.h
 *
 *  Created on: Sep 5, 2023
 *      Author: toasty
 */

#include "Button_Driver.h"
#include <stdint.h> // "be sure to include whichever library is needed for uintx_t support"

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

uint32_t getScheduledEvents();

void addSchedulerEvent(uint32_t);

void removeSchedulerEvent(uint32_t);

#define LED_TOGGLE_BIT0 1<<0 	// 0b1
#define LED_DELAY_BIT1 1<<1	// 0b10
#define POLL_BUTTON 1<<2

#define LCD_GAME_TIME 1<<3


// "remember to shift it one more bit to the left than the previous event" s


#endif /* SCHEDULER_H_ */
