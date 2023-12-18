/*
 * RNG_Driver.c
 *
 *  Created on: Dec 5, 2023
 *      Author: toasty
 */


#include "RNG_Driver.h"

RNG_HandleTypeDef RNGConfig = {0};

void RNG_init(){

	RNGConfig.Instance = RNG;
	__HAL_RCC_RNG_CLK_ENABLE();
	// HAL RNG init
	// HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng);
	HAL_RNG_Init(&RNGConfig);

//	button_clockEnable();
//	HAL_GPIO_Init(GPIOA, &buttonConfig);


}

uint32_t RNG_getNum(){

	uint32_t randomNumber;
	// HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *hrng, uint32_t *random32bit);
	HAL_RNG_GenerateRandomNumber(&RNGConfig, &randomNumber);

	return randomNumber;

}
