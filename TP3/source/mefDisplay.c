/*
 * mefDisplay.c
 *
 *  Created on: 4 jun. 2023
 *      Author: rmaero
 */
#include "mma8451.h"
#include "display.h"

#define TIMER_1S 1000

static uint16_t displayTimer;

void mefDisplay()
{
	if(displayTimer==0)
	{
		displayTimer = TIMER_1S;
		int16_t accX = mma8451_getAcX();
		int16_t accY = mma8451_getAcY();
		int16_t accZ = mma8451_getAcZ();
		display_reading(accX,accY,accZ);
	}
}

void mefDisplay_task1ms()
{
	if(displayTimer)
		displayTimer--;
}
