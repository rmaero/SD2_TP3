#include <mefRecTrama.h>
#include "SD2_board.h"
#include "board.h"
#include "mma8451.h"
#include "uart_ringBuffer.h"
#include "procTrama.h"
#include "enviarAccContinuo.h"

int main(void)
{
	BOARD_BootClockRUN();

	// Se inicializan funciones de la placa
	board_init();

	//inicializa interrupción de systick cada 1 ms
	SysTick_Config(SystemCoreClock / 1000U);

	mma8451_setDR_int();
	uart_ringBuffer_init();

    while(1)
    {
       mefRecTrama_task();
       //enviarContinuo();
    }
}

void SysTick_Handler(void)
{
	enviarAccContinuo_1ms();
}
