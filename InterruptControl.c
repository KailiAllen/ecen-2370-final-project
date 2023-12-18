/*
 * InterruptControl.c
 *
 *  Created on: Oct 10, 2023
 *      Author: toasty
 */

#include "InterruptControl.h"


void IRQ_enable(uint8_t irq){
	if(irq < 32){
		// set the proper bit in the proper NVIC register by shifting bitmask by the irq number
		*NVIC_ISER0 |= 1<<irq;
	}
	else if (irq < 64){ // this is effectively the same as irq >= 32 for our purposes
		*NVIC_ISER1 |= 1<<(irq % 32); // check
	}
}

void IRQ_disable(uint8_t irq){
	if(irq < 32){

		*NVIC_ICER0 |= 1<<irq;
	}
	else if (irq < 64){
		*NVIC_ICER1 |= 1<<(irq % 32);
	}
}

void IRQ_clear_pending(uint8_t irq){
	if(irq < 32){

		*NVIC_ICPR0 |= 1<<irq;
	}
	else if (irq < 64){
		*NVIC_ICPR1 |= 1<<(irq % 32);
	}
}

void IRQ_set_pending(uint8_t irq){

	if(irq < 32){

		*NVIC_ISPR0 |= 1<<irq;
	}
	else if (irq < 64){
		*NVIC_ISPR1 |= 1<<(irq % 32);
	}
}

void EXTI_clearPendingInterrupt(uint8_t pin){
	// set the proper bit in the pending regis(ter)? of the EXTI peripheral
	// doc says "This bit is cleared by programming it to ‘1’."
	EXTI_1->EXTI_PR |= 1<<pin;

}
