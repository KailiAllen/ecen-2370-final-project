/*
 * RNG_Driver.h
 *
 *  Created on: Dec 5, 2023
 *      Author: toasty
 */

#ifndef INC_RNG_DRIVER_H_
#define INC_RNG_DRIVER_H_

#include "stm32f4xx_hal.h"
//#include "ApplicationCode.h"

void RNG_init();
uint32_t RNG_getNum();



#endif /* INC_RNG_DRIVER_H_ */
