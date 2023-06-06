
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


#include "SD2_board.h"
#include "fsl_lpsci.h"
#include "fsl_port.h"
#include "board.h"
#include "MKL46Z4.h"
#include "pin_mux.h"
#include "ringBuffer.h"

static void* pRingBufferRx;
static void* pRingBufferTx;


void uart_ringBuffer_init(void)
{
	lpsci_config_t config;

    pRingBufferRx = ringBuffer_init(16);
    pRingBufferTx = ringBuffer_init(32);//NOTA: LO TUVE QUE INCREMENTAR para que no salga la trama del acelerometro cortada

	CLOCK_SetLpsci0Clock(0x1U);

	/* PORTA1 (pin 35) is configured as UART0_RX */
	PORT_SetPinMux(PORTA, 1U, kPORT_MuxAlt2);

	/* PORTA2 (pin 36) is configured as UART0_TX */
	PORT_SetPinMux(PORTA, 2U, kPORT_MuxAlt2);

	/*
	 * config.parityMode = kLPSCI_ParityDisabled;
	 * config.stopBitCount = kLPSCI_OneStopBit;
	 * config.enableTx = false;
	 * config.enableRx = false;
	 */
	LPSCI_GetDefaultConfig(&config);
	config.baudRate_Bps = 115200;
	config.parityMode = kLPSCI_ParityDisabled;
	config.stopBitCount = kLPSCI_OneStopBit;
	config.enableTx = true;
	config.enableRx = true;

	LPSCI_Init(UART0, &config, CLOCK_GetFreq(kCLOCK_CoreSysClk));

	/* Habilita interrupciones */
	LPSCI_EnableInterrupts(UART0, kLPSCI_RxDataRegFullInterruptEnable);
	LPSCI_EnableInterrupts(UART0, kLPSCI_TxDataRegEmptyInterruptEnable);

	LPSCI_EnableInterrupts(UART0,kLPSCI_RxOverrunFlag );
	LPSCI_EnableInterrupts(UART0,kLPSCI_FramingErrorFlag );
	EnableIRQ(UART0_IRQn);
}

/** \brief recibe datos por puerto serie accediendo al RB
 **
 ** \param[inout] pBuf buffer a donde guardar los datos
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes recibidos
 **/
int32_t uart_ringBuffer_recDatos(uint8_t *pBuf, int32_t size)
{
    int32_t ret = 0;

    /* entra sección de código crítico */
    NVIC_DisableIRQ(UART0_IRQn);

    while (!ringBuffer_isEmpty(pRingBufferRx) && ret < size)
    {
    	uint8_t dato;

        ringBuffer_getData(pRingBufferRx, &dato);

        pBuf[ret] = dato;

        ret++;
    }

    /* sale de sección de código crítico */
    NVIC_EnableIRQ(UART0_IRQn);

    return ret;
}

/** \brief envía datos por puerto serie accediendo al RB
 **
 ** \param[inout] pBuf buffer a donde estan los datos a enviar
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes enviados
 **/
int32_t uart_ringBuffer_envDatos(uint8_t *pBuf, int32_t size)
{
    int32_t ret = 0;

    /* entra sección de código crítico */
    NVIC_DisableIRQ(UART0_IRQn);

    /* si el buffer estaba vacío hay que habilitar la int TX */
    if (ringBuffer_isEmpty(pRingBufferTx))
    	LPSCI_EnableInterrupts(UART0, kLPSCI_TxDataRegEmptyInterruptEnable);

    while (!ringBuffer_isFull(pRingBufferTx) && ret < size)
    {
        ringBuffer_putData(pRingBufferTx, pBuf[ret]);
        ret++;
    }

    /* sale de sección de código crítico */
    NVIC_EnableIRQ(UART0_IRQn);

    return ret;
}


void UART0_IRQHandler(void)
{
	uint8_t data;

	//============ DEBUG SE CUELGA EN COMM CONTINUA =================
	if ( kLPSCI_RxOverrunFlag & LPSCI_GetStatusFlags(UART0))// &&
			//(kLPSCI_RxOverrunInterruptEnable) & LPSCI_GetEnabledInterrupts(UART0) )
	{
		board_setLed(BOARD_LED_ID_ROJO, BOARD_LED_MSG_ON);//TODO DISABLE THIS, FOR DEBUG PURPOSES ONLY
		LPSCI_ClearStatusFlags(UART0, kLPSCI_RxOverrunFlag);
	}
	//===========DEBUG=========================================

    if ( (kLPSCI_RxDataRegFullFlag)            & LPSCI_GetStatusFlags(UART0) &&
         (kLPSCI_RxDataRegFullInterruptEnable) & LPSCI_GetEnabledInterrupts(UART0) )
	{
        /* obtiene dato recibido por puerto serie */
	    data = LPSCI_ReadByte(UART0);

		/* pone dato en ring buffer */
		ringBuffer_putData(pRingBufferRx, data);

		LPSCI_ClearStatusFlags(UART0, kLPSCI_RxDataRegFullFlag);
	}

	if ( (kLPSCI_TxDataRegEmptyFlag)            & LPSCI_GetStatusFlags(UART0) &&
         (kLPSCI_TxDataRegEmptyInterruptEnable) & LPSCI_GetEnabledInterrupts(UART0) )
	{
		if (ringBuffer_getData(pRingBufferTx, &data))
		{
			/* envía dato extraído del RB al puerto serie */
		    LPSCI_WriteByte(UART0, data);
		}
		else
		{
			/* si el RB está vacío deshabilita interrupción TX */
		    LPSCI_DisableInterrupts(UART0, kLPSCI_TxDataRegEmptyInterruptEnable);
		}

		LPSCI_ClearStatusFlags(UART0, kLPSCI_TxDataRegEmptyFlag);
	}
}
