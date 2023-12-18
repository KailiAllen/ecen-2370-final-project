/*
 * InterruptControl.h
 *
 *  Created on: Oct 10, 2023
 *      Author: toasty
 */

#include <stdint.h>
#include "stm32f4xx_hal.h"


#ifndef INTERRUPTCONTROL_H_
#define INTERRUPTCONTROL_H_

#define EXTI0_IRQ_NUMBER 6

#define APB2_BASE_ADDR 0x40010000
#define APB1_BASE_ADDR 0x40000000
#define PERIPHERAL_BASE_ADDR 0x40000000
#define AHB1_BASE_ADDR 0x40020000
#define RCC_BASE_ADDR 0x40023800

#define SYSCFG_BASE_ADDR (APB2_BASE_ADDR + 0x00003800)
#define EXTI_BASE_ADDR (APB2_BASE_ADDR + 0x00003C00)

#define NVIC_ISER0 ((volatile uint32_t*) 0xE000E100)
#define NVIC_ICER0 ((volatile uint32_t*) 0xE000E180)
#define NVIC_ISPR0 ((volatile uint32_t*) 0xE000E200)
#define NVIC_ICPR0 ((volatile uint32_t*) 0xE000E280)
#define NVIC_IPR0  ((volatile uint32_t*) 0xE000E400)
// NVIC_IPR1 would be 0xE000E404 because if uint32_t is ptr, ptr++ adds the 4

#define NVIC_ISER1 ((volatile uint32_t*) 0xE000E104)
#define NVIC_ICER1 ((volatile uint32_t*) 0xE000E184)
#define NVIC_ISPR1 ((volatile uint32_t*) 0xE000E204)
#define NVIC_ICPR1 ((volatile uint32_t*) 0xE000E284)

typedef struct{
	volatile uint32_t EXTI_IMR; 	// interrupt mask
	volatile uint32_t EXTI_EMR;		// event mask
	volatile uint32_t EXTI_RSTR; 	// rising trigger selection
	volatile uint32_t EXTI_FSTR;	// falling trigger selection
	volatile uint32_t EXTI_SWIER;	// software interrupt event
	volatile uint32_t EXTI_PR;		// pending
}EXTI_RegDef_t;

#define EXTI_1 ((EXTI_RegDef_t*) EXTI_BASE_ADDR)

void IRQ_enable(uint8_t irq);
void IRQ_disable(uint8_t irq);
void IRQ_clear_pending(uint8_t irq);
void IRQ_set_pending(uint8_t irq);

void EXTI_clearPendingInterrupt(uint8_t pin);

#endif /* INTERRUPTCONTROL_H_ */
