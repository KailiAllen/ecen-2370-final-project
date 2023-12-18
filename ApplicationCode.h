/*
 * ApplicationCode.h
 *
 *  Created on: Nov 14, 2023
 *      Author: xcowa
 */

#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

#include "LCD_Driver.h"
//#include "RNG_Driver.h"
//#include "Button_Driver.h"
#include "Scheduler.h"
//#include "Timer_Driver.h"
//#include "ErrorHandling.h"

void ApplicationInit(void);
void RunDemoForLCD(void);
void testLCD();
//void lvl1();

void delayLED(uint32_t);

// * BUTTON INIT:

// "an application level button init function"
void app_button_init();
void executeButtonPollingRoutine();
#define USE_INTERRUPT_FOR_BUTTON 1
void app_button_interrupt_init();

void app_run_game();

void app_reactiontime();

bool app_getpressed(bool pressed);

void EXTI0_IRQHandler();

#endif /* INC_APPLICATIONCODE_H_ */
