#include <mefRecTrama.h>
#include "SD2_board.h"
#include "board.h"
#include "mma8451.h"
#include "uart_ringBuffer.h"
#include "procTrama.h"
#include "enviarAccContinuo.h"
#include "oled.h"
#include "display.h"
#include "mefDisplay.h"

int main(void)
{
	BOARD_BootClockRUN();

	// Se inicializan funciones de la placa
	board_init();

	//inicializa interrupción de systick cada 1 ms
	SysTick_Config(SystemCoreClock / 1000U);

	//I2C ACCELEROMETER
	mma8451_setDR_int();

	//UART0
	uart_ringBuffer_init();

	//SPI Display
	display_init();
	display_header();

    while(1)
    {
       mefRecTrama_task();
       mefDisplay();
       //enviarContinuo();
    }
}

void SysTick_Handler(void)
{
	enviarAccContinuo_1ms();
	mefDisplay_task1ms();
}
