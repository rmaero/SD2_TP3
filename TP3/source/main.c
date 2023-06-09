#include "board.h"
#include "SD2_board.h"
#include <uart0_DMA.h>
#include "mma8451.h"
#include "oled.h"
#include "uart1.h"
#include <mefRecTrama.h>
#include "procTrama.h"
#include "display.h"
#include "mefDisplay.h"
#include "enviarAccContinuo.h"

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
	uart0_init();
	uart1_init();

	//SPI Display
	display_init();
	display_header();

    while(1)
    {
    	mefRecTrama_task();//UART1
    	mefDisplay();
    	enviarContinuo();//UART0
    }
}

void SysTick_Handler(void)
{
	enviarAccContinuo_1ms();
	mefDisplay_task1ms();
	uart1_periodicTask1ms();
}
