/*
 * Button_Driver.h
 *
 *  Created on: Sep 26, 2023
 *      Author: toasty
 */

#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "InterruptControl.h"

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_

// Find the port and pin number for the User Button.
// User Button Port: PA0

#define BUTTON_PORT_VALUE 	GPIOA
#define BUTTON_PIN_NUMBER 	0
#define BUTTON_PRESSED 		1  			// 1 means pressed, 0 means not
#define BUTTON_UNPRESSED 	0

void button_init();
void clockEnable();
void clockDisable(); 		// "may never get used"
bool isButtonPressed();  	// if pressed return 1, if unpressed return 0

void button_interrupt_init();

// when you get to the struct part: inside your initializeButton() function,
// make a buttonConfig handle_t struct that is all set to 0
/* initializeButton(){
 * 	whatevertypeHandle_t ButtonConfig = {0};
 * 	}
 */



#endif /* BUTTON_DRIVER_H_ */
