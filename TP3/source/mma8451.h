
#ifndef MMA8451_H_
#define MMA8451_H_


#include "stdint.h"
#include "stdbool.h"


#ifdef __cplusplus
extern "C" {
#endif



typedef enum
{
    DR_800hz = 0b000,
    DR_400hz = 0b001,
    DR_200hz = 0b010,
    DR_100hz = 0b011,
    DR_50hz = 0b100,
    DR_12p5hz = 0b101,
    DR_6p25hz = 0b110,
    DR_1p56hz = 0b111,
}DR_enum;

/** \brief puerto I2C utilizado en el acelerómetro  */
#define MMA8451_I2C     I2C0

/** \brief configura aceler�metro MMA8451
 **	
 **/
void mma8451_init(void);

/** \brief Lee lectura del aceler�metro en el eje X
 **
 ** \return Lectura del aceler�metro en cent�cimas de g
 **/
int16_t mma8451_getAcX(void);
int16_t mma8451_getAcY(void);
int16_t mma8451_getAcZ(void);

void mma8451_setDataRate(DR_enum rate);

void mma8451_setFF_int(void);
void mma8451_setDR_int(void);

void mma8451_disableInt(void);


bool mma8451_getFFEv();

#ifdef __cplusplus
}
#endif


#endif /* MMA8451_H_ */
