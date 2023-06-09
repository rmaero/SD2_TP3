/*
 * uart1.c
 *
 *  Created on: 8 jun. 2023
 *      Author: Rodrigo
 */


#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "board.h"
#include "MKL46Z4.h"
#include "pin_mux.h"
#include "fsl_uart.h"
#include "fsl_port.h"
#include "SD2_board.h"
#include "ringBuffer.h"
//DMA

#include "fsl_uart_dma.h"
#include "fsl_dmamux.h"


#define UART1_TX_DMA_CHANNEL 1U //Uso el segundo canal de DMA para la uart1
#define UART1_TX_BUFFER_DMA_SIZE  32

#define RS485_UART                  UART1
#define RS485_UART_IRQn             UART1_IRQn
#define RS485_kSimClockGateUart     kSimClockGateUart1

#define UART1DELAY_MS 2
static uint32_t uart1Delay;

static void* uart1_pRingBufferRx;

static uint8_t txBuffer_dma[UART1_TX_BUFFER_DMA_SIZE];
//static lpsci_dma_handle_t lpsciDmaHandle; //?
static uart_dma_handle_t uart1DmaHandle;
static dma_handle_t uart1TxDmaHandle;

static uint8_t byteRec;
volatile bool uart1TxOnGoing = false;

/* UART user callback */
static void UART1_UserCallback(UART_Type *base, uart_dma_handle_t *handle, status_t status, void *userData)
{

	if (kStatus_UART_TxIdle == status)
	{
		uart1TxOnGoing = false;
	}

	uart1Delay=UART1DELAY_MS;//espera X milis despues de terminar la transaccion para bajar DE y RE

}

void uart1_init()
{
	uart1_pRingBufferRx = ringBuffer_init(16);
	uart_config_t config;
	UART_GetDefaultConfig(&config);
	config.baudRate_Bps = 115200;
	config.enableTx = true;
	config.enableRx = true;

	UART_Init(RS485_UART, &config, CLOCK_GetFreq(BUS_CLK));

	/* Configura los pines RX y TX de la UART1 */
	PORT_SetPinMux(PORTE, 0U, kPORT_MuxAlt3);//Tx del micro
	PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt3);//Rx del micro



	UART_EnableInterrupts(RS485_UART, kUART_RxDataRegFullInterruptEnable);
	UART_EnableInterrupts(RS485_UART, kUART_TxDataRegEmptyInterruptEnable);
	UART_EnableInterrupts(RS485_UART, kUART_TransmissionCompleteInterruptEnable);
	UART_EnableInterrupts(RS485_UART,kUART_RxOverrunFlag );
	UART_EnableInterrupts(RS485_UART,kUART_FramingErrorFlag );
	NVIC_EnableIRQ(RS485_UART_IRQn);
	/*
	 * DMA
	 */
    /* Init DMAMUX */
	DMAMUX_Init(DMAMUX0);//hace falta inicializarlo? si lo inicializa UART0

	/* Set channel for UART1  */
	DMAMUX_SetSource(DMAMUX0, UART1_TX_DMA_CHANNEL, kDmaRequestMux0UART1Tx);
	DMAMUX_EnableChannel(DMAMUX0, UART1_TX_DMA_CHANNEL);

	/* Init the DMA module */
	DMA_Init(DMA0);//hace falta inicializarlo? si lo inicializa UART0
	DMA_CreateHandle(&uart1TxDmaHandle, DMA0, UART1_TX_DMA_CHANNEL);

	/* Create UART1 DMA handle. */
	UART_TransferCreateHandleDMA(
			UART1,
			&uart1DmaHandle,
			UART1_UserCallback,
			NULL,
			&uart1TxDmaHandle,
			NULL);
	/*============== CONFIGURACIÓN DMA (sólo para TX)============= */
	rs485_RE(false);
	rs485_DE(false);
}



/** \brief recibe datos por puerto serie accediendo al RB
 **
 ** \param[inout] pBuf buffer a donde guardar los datos
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes recibidos
 **/
int32_t uart1_ringBuffer_recDatos(uint8_t *pBuf, int32_t size)
{
    int32_t ret = 0;

    /* entra sección de código crítico */
    NVIC_DisableIRQ(UART1_IRQn);

    while (!ringBuffer_isEmpty(uart1_pRingBufferRx) && ret < size)
    {
    	uint8_t dato;

        ringBuffer_getData(uart1_pRingBufferRx, &dato);

        pBuf[ret] = dato;

        ret++;
    }

    /* sale de sección de código crítico */
    NVIC_EnableIRQ(UART1_IRQn);

    return ret;
}

/** \brief envía datos por puerto serie accediendo a memoria RAM
 **
 ** \param[inout] pBuf buffer a donde estan los datos a enviar
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes enviados
 **/
int32_t uart1_DMA_envDatos(uint8_t *pBuf, int32_t size)
{
	uart_transfer_t xfer;

    if (uart1TxOnGoing)
    {
        size = 0;
    }else
    {
        /* limita size *///recorta lo que se pase
        if (size > UART1_TX_BUFFER_DMA_SIZE)
            size = UART1_TX_BUFFER_DMA_SIZE;

        // Hace copia del buffer a transmitir en otro arreglo
        memcpy(txBuffer_dma, pBuf, size);

        xfer.data = txBuffer_dma;
        xfer.dataSize = size;

        uart1TxOnGoing = true;
    	rs485_RE(true);
    	rs485_DE(true);

        UART_TransferSendDMA(RS485_UART, &uart1DmaHandle, &xfer);

        UART_EnableInterrupts(RS485_UART, kUART_TransmissionCompleteInterruptEnable);


    }

    return size;
}
//TODO
void UART1_IRQHandler(void)
{
	//es necesario?
	if ( kUART_RxOverrunFlag & UART_GetStatusFlags(RS485_UART))// &&
			//(kUART_RxOverrunInterruptEnable) & UART_GetEnabledInterrupts(RS485_UART) )
	{
		UART_ClearStatusFlags(UART1, kUART_RxOverrunFlag);
	}

	if ( (kUART_RxDataRegFullFlag)            & UART_GetStatusFlags(RS485_UART) &&
	     (kUART_RxDataRegFullInterruptEnable) & UART_GetEnabledInterrupts(RS485_UART) )
	{
		/* obtiene dato recibido por puerto serie */
		byteRec = UART_ReadByte(RS485_UART);
		/* pone dato en ring buffer */
		ringBuffer_putData(uart1_pRingBufferRx, byteRec);
		UART_ClearStatusFlags(RS485_UART, kUART_RxDataRegFullFlag);
	}

	if ( (kUART_TxDataRegEmptyFlag)            & UART_GetStatusFlags(RS485_UART) &&
		 (kUART_TxDataRegEmptyInterruptEnable) & UART_GetEnabledInterrupts(RS485_UART) )
    {
        /* entra acá cuando se se puede poner un nuevo byte en el buffer
         * de transmición
         */
		UART_DisableInterrupts(RS485_UART, kUART_TxDataRegEmptyInterruptEnable);
		UART_ClearStatusFlags(RS485_UART, kUART_TxDataRegEmptyFlag);
    }

	if ( (kUART_TransmissionCompleteFlag)            & UART_GetStatusFlags(RS485_UART) &&
		 (kUART_TransmissionCompleteInterruptEnable) & UART_GetEnabledInterrupts(RS485_UART) )
    {
        /* entra acá cuando se completó la transmisión del byte
         *
         */
		UART_DisableInterrupts(RS485_UART, kUART_TransmissionCompleteInterruptEnable);
		UART_ClearStatusFlags(RS485_UART, kUART_TransmissionCompleteFlag);
    }
}

void uart1_periodicTask1ms()
{
	if(uart1Delay)
	{
		uart1Delay--;
	}else{//medio sucio esto, delay para que no baje DE y RE instantaneamente
		rs485_RE(false);
		rs485_DE(false);
	}

}
