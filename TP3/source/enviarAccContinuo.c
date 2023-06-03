#include "SD2_board.h"
#include "mma8451.h"

/*
 * enviarAccContinuo.c
 *
 *  Created on: 3 jun. 2023
 *      Author: rmaero
 */
#define TIMER_150MS 150
uint32_t timer=0;

void enviarContinuo()
{
	if(timer==0)
	{
		timer=TIMER_150MS;
		char tramaRespuestaAcc[15];
			int16_t accX =mma8451_getAcX();
			int16_t accY =mma8451_getAcY();
			int16_t accZ =mma8451_getAcZ();
			char signX='+',signY='+',signZ='+';
			if(accX<0)
				signX='-';
			if(accY<0)
				signY='-';
			if(accZ<0)
				signZ='-';
			//responder
			sprintf(&tramaRespuestaAcc,"%c%03d %c%03d %c%03d\n", signX,abs(accX), signY,abs(accY), signZ,abs(accZ) );
			uart_ringBuffer_envDatos(&tramaRespuestaAcc,sizeof(tramaRespuestaAcc));
	}
}
void enviarAccContinuo_1ms()
{
	if(timer)
		timer--;
}
