#include "stdlib.h"
#include "stdio.h"

#include "procTrama.h"
#include "uart_ringBuffer.h"
#include "SD2_board.h"
#include "mma8451.h"

#define NUMERO_GRUPO 16
//bytes de trama recibidos
#define BYTE0 0
#define BYTE1 1
#define BYTE2 2
#define BYTE3 3
#define BYTE4 4
#define BYTE5 5


static void accionLed(uint8_t charIdLed, board_ledMsg_enum ledMsg)
{
	switch (charIdLed)
	{
		case '1':
			board_setLed(BOARD_LED_ID_ROJO, ledMsg);
			break;

		case '2':
			board_setLed(BOARD_LED_ID_VERDE, ledMsg);
			break;
	}
}
static void procLed(char *buf)
{
	char tramaRespuesta[7];
	switch(buf[BYTE4])
	{
		case 'A':
			accionLed(buf[BYTE3], BOARD_LED_MSG_OFF);
			sprintf(&tramaRespuesta,":%02d0%cA\n",NUMERO_GRUPO,buf[BYTE3]);
			break;
		case 'E':
			accionLed(buf[BYTE3], BOARD_LED_MSG_ON);
			sprintf(&tramaRespuesta,":%02d0%cE\n",NUMERO_GRUPO,buf[BYTE3]);
			break;
		case 'T':
			accionLed(buf[BYTE3], BOARD_LED_MSG_TOGGLE);
			sprintf(&tramaRespuesta,":%02d0%cT\n",NUMERO_GRUPO,buf[BYTE3]);
			break;
		default:
			sprintf(&tramaRespuesta,"ERROR\n");
			break;
	}
	//Responder igual a la peticion
	//Tx por ringbuffer
	//uart_ringBuffer_envDatos(&tramaRespuesta,sizeof(tramaRespuesta));
	uart0_DMA_envDatos(&tramaRespuesta,sizeof(tramaRespuesta));
}

static void procSw(char *buf)
{
	bool estadoSw;
	char tramaRespuesta[7];
	switch(buf[BYTE3])
	{
		case '1':
			estadoSw=board_getSw(BOARD_SW_ID_1);
			//devolver el estado del SW1
			if (estadoSw)
				sprintf(&tramaRespuesta,":%02d11P\n",NUMERO_GRUPO);
			else
				sprintf(&tramaRespuesta,":%02d11N\n",NUMERO_GRUPO);
			break;
		case '3':
			estadoSw=board_getSw(BOARD_SW_ID_3);
			//devolver el estado del SW3
			if (estadoSw)
				sprintf(&tramaRespuesta,":%02d13P\n",NUMERO_GRUPO);
			else
				sprintf(&tramaRespuesta,":%02d13N\n",NUMERO_GRUPO);
			break;
		default:
			sprintf(&tramaRespuesta,"ERROR\n");
			break;
	}
	//Tx por ringbuffer
	//uart_ringBuffer_envDatos(&tramaRespuesta,sizeof(tramaRespuesta));
	//Tx por DMA
	uart0_DMA_envDatos(&tramaRespuesta,sizeof(tramaRespuesta));
}

static void procAcc()
{
	char tramaRespuestaAcc[18];
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
	sprintf(&tramaRespuestaAcc,":%02d21%c%03d%c%03d%c%03d\n",NUMERO_GRUPO, signX,abs(accX), signY,abs(accY), signZ,abs(accZ) );
	//Transmite por ringbuffer
	//uart_ringBuffer_envDatos(&tramaRespuestaAcc,sizeof(tramaRespuestaAcc));
	//Transmite por DMA
	uart0_DMA_envDatos(&tramaRespuestaAcc,sizeof(tramaRespuestaAcc));
}

void procTrama(char *buf, int length)
{
	//leds->:XX01Y’LF’
	char tramaAux[7];
	char grupoStr[2];
	grupoStr[BYTE0]=buf[BYTE0];
	grupoStr[BYTE1]=buf[BYTE1];

	//obligadamente le tengo que dar un puntero en el segundo argumento,
	int grupo;
	grupo = (int) strtol(grupoStr, tramaAux, 10);

	if(grupo!=NUMERO_GRUPO)
		return;

    switch (buf[BYTE2])
    {
		case '0'://0= acciones sobre LEDs
			procLed(buf);
			break;

		case '1'://1= leer estado SW
			procSw(buf);
			break;

		case '2'://2= leer acelerometro
			procAcc();
			break;
    }
}
