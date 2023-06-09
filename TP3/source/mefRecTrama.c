#include "uart1.h"
#include "mefRecTrama.h"
#include "procTrama.h"


typedef enum
{
	MEF_REC_ESPERANDO_INICIO = 0,
	MEF_REC_RECIBIENDO,
}mefRecTrama_estado_enum;

#define BUFFER_SIZE		8

#define CHAR_CR			0xd
#define CHAR_LF			0xa

static char bufferRec[BUFFER_SIZE];

void mefRecTrama_task(void)
{
	static mefRecTrama_estado_enum estado = MEF_REC_ESPERANDO_INICIO;

	uint32_t flagRec;
	uint8_t byteRec;
	static uint8_t indexRec;

	flagRec = uart1_ringBuffer_recDatos(&byteRec, sizeof(byteRec));

	switch (estado)
	{
		case MEF_REC_ESPERANDO_INICIO:
			if (flagRec != 0 && byteRec == ':')
			{
				indexRec = 0;
				estado = MEF_REC_RECIBIENDO;
			}
			break;

		case MEF_REC_RECIBIENDO:
			if (flagRec != 0 && byteRec != CHAR_LF)
			{
				if (indexRec < BUFFER_SIZE)
					bufferRec[indexRec] = byteRec;
				indexRec++;
			}

			if (flagRec != 0 && byteRec == ':')
				indexRec = 0;

			if (flagRec != 0 && byteRec == CHAR_LF)
			{
				procTrama(bufferRec, indexRec);
				estado = MEF_REC_ESPERANDO_INICIO;
			}

			if (indexRec > BUFFER_SIZE )
			{
				estado = MEF_REC_ESPERANDO_INICIO;
			}
			break;
	}
}
