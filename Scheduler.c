/*
 * Scheduler.c
 *
 *  Created on: Sep 5, 2023
 *      Author: toasty
 */

#include "Scheduler.h"

static uint32_t scheduledEvents;

void addSchedulerEvent(uint32_t var){

	scheduledEvents |= var;
}

void removeSchedulerEvent(uint32_t var){

	scheduledEvents &= ~var;
}

uint32_t getScheduledEvents(){

	return scheduledEvents;
}
