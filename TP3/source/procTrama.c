#include "stdlib.h"
#include "procTrama.h"
#include "uart_ringBuffer.h"
#include "SD2_board.h"

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



void procTrama(char *buf, int length)
{
	//:XX01Y’LF’
	char tramaRespuesta[7];//maybe unnecesary
	char tramaRespuestaAcc[18];
	bool estadoSw;
	char grupoStr[2];
	grupoStr[BYTE0]=buf[BYTE0];
	grupoStr[BYTE1]=buf[BYTE1];

	int grupo;
	grupo = (int) strtol(grupoStr, tramaRespuesta, 10); //obligadamente le tengo que dar un puntero en el segundo argumento, es basura pero no importa porque lo voy a pisar luego.

	if(grupo!=NUMERO_GRUPO)
		return;



    switch (buf[BYTE2])
    {
		case '0'://0= acciones sobre LEDs
			switch(buf[BYTE4])
			{
				case 'A':
					accionLed(buf[BYTE3], BOARD_LED_MSG_OFF);
					break;
				case 'P':
					accionLed(buf[BYTE3], BOARD_LED_MSG_ON);
					break;
				case 'T':
					accionLed(buf[BYTE3], BOARD_LED_MSG_TOGGLE);
					break;
				default:
					//DEVOLVER ERROR
					break;
			}
			//Responder igual a la peticion
			uart_ringBuffer_envDatos(&buf,sizeof(buf));
			break;

		case '1'://1= leer estado SW
			switch(buf[BYTE4])
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
					//DEVOLVER ERROR
					break;
			}
			uart_ringBuffer_envDatos(&tramaRespuesta,sizeof(tramaRespuesta));
			break;

		case '2'://2= leer acelerometro
			//responder
			//TODO corregir signo
			sprintf(&tramaRespuestaAcc,":%02d21+%03d+%03d+%03d\n",NUMERO_GRUPO);
			uart_ringBuffer_envDatos(&tramaRespuestaAcc,sizeof(tramaRespuestaAcc));
			break;

    }
}
