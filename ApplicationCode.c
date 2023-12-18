/*
 * ApplicationCode.c
 *
 *  Created on: Nov 14, 2023
 *      Author: xcowa
 */

#include "ApplicationCode.h"

void ApplicationInit(void)
{
	button_interrupt_init();
	LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);
    RNG_init();

    addSchedulerEvent(LCD_GAME_TIME);

}

void RunDemoForLCD(void)
{
	QuickDemo();
}

void testLCD(){
	testingtesting();
}

void app_run_game(){
	run_game();
}

void app_reactiontime(uint32_t time){
	reactiontime(time);
}

bool app_getpressed(bool pressed){
	return pressed;
}

void EXTI0_IRQHandler(){

	IRQ_disable(EXTI0_IRQ_NUMBER);

	setpressed1();

	EXTI_clearPendingInterrupt(BUTTON_PIN_NUMBER);
	IRQ_enable(EXTI0_IRQ_NUMBER);
}
