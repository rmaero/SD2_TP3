#include "procTrama.h"
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
	char grupoStr[2];
	grupoStr[BYTE0]=buf[BYTE0];
	grupoStr[BYTE1]=buf[BYTE1];

	int grupo = (int) strtol(grupo, (char **)NULL, 10);
	if(grupo!=NUMERO_GRUPO)
		break;

	bool estadoSw;

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
				case default:
					//DEVOLVER ERROR
					break;
			}
			//Responder igual a la peticion
			break;

		case '1'://1= leer estado SW
			switch(buf[BYTE4])
			{
				case '1':
					estadoSw=board_getSw(BOARD_SW_ID_1);
					//devolver el estado del SW1
					//TRAMA ":1611P o N
					break;
				case '3':
					estadoSw=board_getSw(BOARD_SW_ID_3);
					//devolver el estado del SW1
					//TRAMA ":1613P o N
					break;
				case default:
					//DEVOLVER ERROR
					break;
			}
			break;

		case '2'://2= leer acelerometro

			break;

    }
}
