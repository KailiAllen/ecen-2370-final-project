/*
 * Button_Driver.c
 *
 *  Created on: Sep 26, 2023
 *      Author: toasty
 */

#include "Button_Driver.h"

 // init to 0 because it's not static :O


void button_clockEnable(){

	__HAL_RCC_GPIOA_CLK_ENABLE();
}

bool isButtonPressed(){

//	uint8_t state = readPortInput(BUTTON_PORT_VALUE, BUTTON_PIN_NUMBER);
	GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, BUTTON_PIN_NUMBER);


	if(state == BUTTON_PRESSED){
		return BUTTON_PRESSED; // return true
	}

	else{
		return BUTTON_UNPRESSED; // return false
	}

}

void button_init(){

	// "this function does not have to be as modular as it would be if we had multiple buttons"
	// aka no switch statement or function input
	GPIO_InitTypeDef buttonConfig = {0};
	buttonConfig.Pin = GPIO_PIN_0;
	buttonConfig.Mode = GPIO_MODE_INPUT;

	button_clockEnable();
	HAL_GPIO_Init(GPIOA, &buttonConfig);


}

void button_interrupt_init(){

	GPIO_InitTypeDef buttonConfig = {0};
	buttonConfig.Pin = GPIO_PIN_0;
	buttonConfig.Mode = GPIO_MODE_IT_RISING;
	buttonConfig.Pull = GPIO_NOPULL;
	buttonConfig.Speed = GPIO_SPEED_FREQ_HIGH;

	button_clockEnable();
	HAL_GPIO_Init(GPIOA, &buttonConfig);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

