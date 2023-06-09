/*
 * uart1.h
 *
 *  Created on: 8 jun. 2023
 *      Author: Rodrigo
 */

#ifndef UART1_H_
#define UART1_H_

#include "stdint.h"
#include "stdbool.h"

void uart1_init(void);

/** \brief recibe datos por puerto serie accediendo al RB
 **
 ** \param[inout] pBuf buffer a donde guardar los datos
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes recibidos
 **/
int32_t uart1_ringBuffer_recDatos(uint8_t *pBuf, int32_t size);

/** \brief envía datos por puerto serie accediendo al RB
 **
 ** \param[inout] pBuf buffer a donde estan los datos a enviar
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes enviados
 **/
//int32_t uart1_ringBuffer_envDatos(uint8_t *pBuf, int32_t size);

/** \brief envía datos por puerto serie vía DMA
 **
 ** \param[inout] pBuf buffer a donde estan los datos a enviar
 ** \param[in] size tamaño del buffer
 ** \return cantidad de bytes enviados
 **/
int32_t uart1_DMA_envDatos(uint8_t *pBuf, int32_t size);

void uart1_periodicTask1ms();

#endif /* UART1_H_ */
