#include "mma8451.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"

#define MMA8451_I2C_ADDRESS     (0x1d)

#define INT1_PORT       PORTC
#define INT1_GPIO       GPIOC
#define INT1_PIN        5

#define RANGE_2G 4096
#define RANGE_4G 2048
#define RANGE_8G 1024

typedef union
{
    struct
    {
        unsigned SRC_DRDY:1;
        unsigned :1;
        unsigned SRC_FF_MT:1;
        unsigned SRC_PULSE:1;
        unsigned SRC_LNDPRT:1;
        unsigned SRC_TRANS:1;
        unsigned SRC_FIFO:1;
        unsigned SRC_ASLP:1;
    };
    uint8_t data;
}INT_SOURCE_t;

typedef union
{
    struct
    {
        unsigned XDR:1;
        unsigned YDR:1;
        unsigned ZDR:1;
        unsigned ZYXDR:1;
        unsigned XOW:1;
        unsigned YOW:1;
        unsigned ZOW:1;
        unsigned ZYXOW:1;
    };
    uint8_t data;
}STATUS_t	;

typedef union
{
    struct
    {
        unsigned ACTIVE:1;
        unsigned F_READ:1;
        unsigned LNOISE:1;
        unsigned DR:3;
        unsigned ASLP_RATE:2;
    };
    uint8_t data;
}CTRL_REG1_t;

#define CTRL_REG1_ADDRESS   0X2A

typedef union
{
    struct
    {
        unsigned INT_EN_DRDY:1;
        unsigned :1;
        unsigned INT_EN_FF_MT:1;
        unsigned INT_EN_PULSE:1;
        unsigned INT_EN_LNDPRT:1;
        unsigned INT_EN_TRANS:1;
        unsigned INT_EN_FIFO:1;
        unsigned INT_EN_ASLP:1;
    };
    uint8_t data;
}CTRL_REG4_t;

#define CTRL_REG4_ADDRESS   0X2D

typedef union
{
    struct
    {
        unsigned INT_CFG_DRDY:1;
        unsigned :1;
        unsigned INT_CFG_FF_MT:1;
        unsigned INT_CFG_PULSE:1;
        unsigned INT_CFG_LNDPRT:1;
        unsigned INT_CFG_TRANS:1;
        unsigned INT_CFG_FIFO:1;
        unsigned INT_CFG_ASLP:1;
    };
    uint8_t data;
}CTRL_REG5_t;

//=============RESOLUTION/RANGE CONFIG===========
//XYZ_DATA_CFG (read/write)
typedef union
{
    struct
    {
        unsigned FS0:1;
        unsigned FS1:1;
        unsigned BIT2:1;
        unsigned BIT3:1;
        unsigned HPF_OUT:1;
        unsigned BIT5:1;
        unsigned BIT6:1;
        unsigned BIT7:1;
    };
    uint8_t data;
}XYZ_DATA_CFG_t;

#define XYZ_DATA_CFG_ADDRESS   0X0E


//Direcciones de registros del acelerometro
#define CTRL_REG5_ADDRESS   0X2E
#define INT_SOURCE_ADDRESS   0X0C



#define STATUS_ADDRESS       0X00
#define OUT_X_MSB_ADDRESS 	0x01
#define ALL_AXIS_REGISTER_SIZE		6

//Posicion de cada byte en el array luego de tomar los 6 registros juntos
#define X_AXIS_MSB 0
#define X_AXIS_LSB 1
#define Y_AXIS_MSB 2
#define Y_AXIS_LSB 3
#define Z_AXIS_MSB 4
#define Z_AXIS_LSB 5



/*********** FREEFALL - MOTION DETECT REGISTERS****************/
//FREEFALL - MOTION CONFIGURATION
typedef union
{
    struct
    {
        unsigned :1;
        unsigned :1;
        unsigned :1;
        unsigned XEFE:1;
        unsigned YEFE:1;
        unsigned ZEFE:1;
        unsigned OAE:1;
        unsigned ELE:1;
    };
    uint8_t data;
}FF_MT_CFG_t;

#define FF_MT_CFG_ADDRESS   0X15

//Motion/Freefall Source Detection Register (Read Only)
typedef union
{
    struct
    {
        unsigned XHP:1;
        unsigned XHE:1;
        unsigned YHP:1;
        unsigned YHE:1;
        unsigned ZHP:1;
        unsigned ZHE:1;
        unsigned :1;
        unsigned EA:1;
    };
    uint8_t data;
}FF_MT_SRC_t;

#define FF_MT_SRC_ADDRESS   0X16

//THRESHOLD REGISTER
typedef union
{
    struct
    {
        unsigned THS0:1;
        unsigned THS1:1;
        unsigned THS2:1;
        unsigned THS3:1;
        unsigned THS4:1;
        unsigned THS5:1;
        unsigned THS6:1;
        unsigned DBCNTM:1;
    };
    uint8_t data;
}FF_MT_THS_t;

#define FF_MT_THS_ADDRESS   0X17

//DEBOUNCE COUNTER REGISTER
typedef union
{
    struct
    {
        unsigned D0:1;
        unsigned D1:1;
        unsigned D2:1;
        unsigned D3:1;
        unsigned D4:1;
        unsigned D5:1;
        unsigned D6:1;
        unsigned D7:1;
    };
    uint8_t data;
}FF_MT_COUNT_t;

#define FF_MT_COUNT_ADDRESS   0X18

static int16_t readX, readY, readZ;
static bool FFint=false;


static uint8_t mma8451_read_reg(uint8_t addr)
{
	i2c_master_transfer_t masterXfer;
    uint8_t ret;

	memset(&masterXfer, 0, sizeof(masterXfer));    // pone todo en cero (sizeof())
	masterXfer.slaveAddress = MMA8451_I2C_ADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = addr;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &ret;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferBlocking(I2C0, &masterXfer);

	return ret;
}

//para leer los 3 ejes al mismo tiempo
static void mma8451_read_multireg(uint8_t addr, uint8_t *buffer, int size )
{
	i2c_master_transfer_t masterXfer;


	memset(&masterXfer, 0, sizeof(masterXfer));    // pone todo en cero (sizeof())
	masterXfer.slaveAddress = MMA8451_I2C_ADDRESS;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = addr;
	masterXfer.subaddressSize = 1;
	masterXfer.data = buffer;
	masterXfer.dataSize = size;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	I2C_MasterTransferBlocking(I2C0, &masterXfer);


}


static void mma8451_write_reg(uint8_t addr, uint8_t data)
{
	i2c_master_transfer_t masterXfer;

    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress = MMA8451_I2C_ADDRESS;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = addr;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &data;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

    I2C_MasterTransferBlocking(I2C0, &masterXfer);
}

//Configura interrupcion del uC
//PIN PTC5, PIN 81 del micro
static void config_port_int1(void)
{
	const port_pin_config_t port_int1_config = {
			/* Internal pull-up/down resistor is disabled */
		.pullSelect = kPORT_PullDisable,
		/* Slow slew rate is configured */
		.slewRate = kPORT_SlowSlewRate,
		/* Passive filter is disabled */
		.passiveFilterEnable = kPORT_PassiveFilterDisable,
		/* Low drive strength is configured */
		.driveStrength = kPORT_LowDriveStrength,
		/* Pin is configured as PTC3 */
		.mux = kPORT_MuxAsGpio,
	};
	const gpio_pin_config_t gpio_int1_config = {
		.pinDirection = kGPIO_DigitalInput,
		.outputLogic = 0U
	};

	PORT_SetPinConfig(INT1_PORT, INT1_PIN, &port_int1_config);
	GPIO_PinInit(INT1_GPIO, INT1_PIN, &gpio_int1_config);

	/* Interrupt polarity active high, or active low. Default value: 0.
	   0: Active low; 1: Active high. VER REGISTRO CTRL_REG3 */
	PORT_SetPinInterruptConfig(INT1_PORT, INT1_PIN, kPORT_InterruptLogicZero);

	NVIC_EnableIRQ(PORTC_PORTD_IRQn);
	NVIC_SetPriority(PORTC_PORTD_IRQn, 0);
}

//deshabilita las interrupciones
void mma8451_disableInt(void)
{
	NVIC_DisableIRQ(PORTC_PORTD_IRQn);
}


void mma8451_init(void)
{
    CTRL_REG1_t ctrl_reg1;
    CTRL_REG4_t ctrl_reg4;
    CTRL_REG5_t ctrl_reg5;
    XYZ_DATA_CFG_t data_cfg_reg;

    //Primero desactivo el acelerómetro, luego escribo otros registros

	ctrl_reg1.ACTIVE = 0;
	ctrl_reg1.F_READ = 0;
	ctrl_reg1.LNOISE = 1;
	ctrl_reg1.DR = DR_12p5hz;
	ctrl_reg1.ASLP_RATE = 0B00;
    mma8451_write_reg(CTRL_REG1_ADDRESS, ctrl_reg1.data);

	ctrl_reg4.INT_EN_DRDY = 0;
	ctrl_reg4.INT_EN_FF_MT = 0;
	ctrl_reg4.INT_EN_PULSE = 0;
	ctrl_reg4.INT_EN_LNDPRT = 0;
	ctrl_reg4.INT_EN_TRANS = 0;
	ctrl_reg4.INT_EN_FIFO = 0;
	ctrl_reg4.INT_EN_ASLP = 0;
	mma8451_write_reg(CTRL_REG4_ADDRESS, ctrl_reg4.data);


	ctrl_reg5.INT_CFG_DRDY = 0;
	ctrl_reg5.INT_CFG_FF_MT = 0;
	ctrl_reg5.INT_CFG_PULSE = 0;
	ctrl_reg5.INT_CFG_LNDPRT = 0;
	ctrl_reg5.INT_CFG_TRANS = 0;
	ctrl_reg5.INT_CFG_FIFO = 0;
	ctrl_reg5.INT_CFG_ASLP = 0;
	mma8451_write_reg(CTRL_REG5_ADDRESS, ctrl_reg5.data);

	//configuro en el FULLSCALE
	//2g -> FS1=0; FS0 = 0
	//4g -> FS1=0; FS0 = 1
	//8g -> FS1=1; FS0 = 0
	data_cfg_reg.FS0=0;
	data_cfg_reg.FS1=0;
	data_cfg_reg.BIT2=0;
	data_cfg_reg.BIT3=0;
	data_cfg_reg.HPF_OUT=0;
	data_cfg_reg.BIT5=0;
	data_cfg_reg.BIT6=0;
	data_cfg_reg.BIT7=0;
	mma8451_write_reg(XYZ_DATA_CFG_ADDRESS, data_cfg_reg.data);


	ctrl_reg1.ACTIVE = 1;
	ctrl_reg1.F_READ = 0;
	ctrl_reg1.LNOISE = 1;
	ctrl_reg1.DR = DR_12p5hz;
	ctrl_reg1.ASLP_RATE = 0B00;
    mma8451_write_reg(CTRL_REG1_ADDRESS, ctrl_reg1.data);

    config_port_int1();
}

//Configura la velocidad de refresco del ADC del acelerometro
void mma8451_setDataRate(DR_enum rate)
{
    CTRL_REG1_t ctr_reg1;
    bool estAct;

    /* antes de modificar data rate es necesario poner ACTIVE = 0 */
    ctr_reg1.data = mma8451_read_reg(CTRL_REG1_ADDRESS);

    /* guarda valor que tiene ACTIVE y luego pone a cero */
    estAct = ctr_reg1.ACTIVE;
    ctr_reg1.ACTIVE = 0;

	mma8451_write_reg(CTRL_REG1_ADDRESS, ctr_reg1.data);

	/* actualiza DR y en la misma escritura va a restaurar ACTIVE */
	ctr_reg1.DR = rate;
	ctr_reg1.ACTIVE = estAct;

	mma8451_write_reg(CTRL_REG1_ADDRESS, ctr_reg1.data);


}

//Devuelve el ultimo valor adquirido de cada eje
int16_t mma8451_getAcX(void)
{
	return (int16_t)(((int32_t)readX * 100) / (int32_t)RANGE_2G);
}

int16_t mma8451_getAcY(void)
{
	return (int16_t)(((int32_t)readY * 100) / (int32_t)RANGE_2G);
}

int16_t mma8451_getAcZ(void)
{
	return (int16_t)(((int32_t)readZ * 100) / (int32_t)RANGE_2G);
}


//ISR maneja interrupciones del acelerometro por DR o FF
void PORTC_PORTD_IRQHandler(void)
{
    int16_t readG;
    INT_SOURCE_t intSource;
    STATUS_t status;
    uint8_t readReg[6];

    intSource.data = mma8451_read_reg(INT_SOURCE_ADDRESS);

    if (intSource.SRC_DRDY)
    {
    	status.data = mma8451_read_reg(STATUS_ADDRESS);
    	//DEBUG: ONE REGISTER AT A TIME
    	/*
    	if (status.XDR)
        {
            readG   = (int16_t)mma8451_read_reg(0x01)<<8;
            readG  |= mma8451_read_reg(0x02);
            readX = readG >> 2;
        }

        if (status.YDR)
        {
            readG   = (int16_t)mma8451_read_reg(0x03)<<8;
            readG  |= mma8451_read_reg(0x04);
            readY = readG >> 2;
        }

        if (status.ZDR)
        {
            readG   = (int16_t)mma8451_read_reg(0x05)<<8;
            readG  |= mma8451_read_reg(0x06);
            readZ = readG >> 2;
        }

        */
        //---------------ALL AXIS AT THE SAME TIME----------
        if (status.ZYXDR)
        {
        	mma8451_read_multireg(OUT_X_MSB_ADDRESS, &readReg[0], ALL_AXIS_REGISTER_SIZE);
            readG   = (int16_t)readReg[X_AXIS_MSB]<<8;
            readG  |= readReg[X_AXIS_LSB];
            readX = readG >> 2;

            readG   = (int16_t)readReg[Y_AXIS_MSB]<<8;
            readG  |= readReg[Y_AXIS_LSB];
            readY = readG >> 2;

            readG   = (int16_t)readReg[Z_AXIS_MSB]<<8;
            readG  |= readReg[Z_AXIS_LSB];
            readZ = readG >> 2;
        }
    }else if (intSource.SRC_FF_MT)
    {
    	//Read the Motion/Freefall Function to clear the interrupt
    	readReg[0]=mma8451_read_reg(FF_MT_SRC_ADDRESS);
    	FFint=true;
    }

    PORT_ClearPinsInterruptFlags(INT1_PORT, 1<<INT1_PIN);
}

bool mma8451_getFFEv()
{
	bool ret = false;
	if(FFint)
	{
		ret= true;
		FFint=false;
	}
	return ret;
}


//Configura el acelerometro para interrumpir por FREEFALL
void mma8451_setFF_int(void)
{

    CTRL_REG1_t ctrl_reg1;
    CTRL_REG4_t ctrl_reg4;
    CTRL_REG5_t ctrl_reg5;


    FF_MT_CFG_t ffCfgReg;//FF_MT_CFG_ADDRESS   0X15
    FF_MT_THS_t ffThsReg;//FF_MT_THS_ADDRESS   0X17
    /*
    FF_MT_SRC_t ffSrcReg;//FF_MT_SRC_ADDRESS   0X16

    FF_MT_COUNT_t debounceCounterReg;//FF_MT_COUNT_ADDRESS   0X18
     */

//step 1 app note
    /* Primero desactivo el acelerómetro, pongo en 50Hz, luego escribo otros registros*/

	ctrl_reg1.ACTIVE = 0;
	ctrl_reg1.F_READ = 0;
	ctrl_reg1.LNOISE = 1;
	ctrl_reg1.DR = DR_50hz;
	ctrl_reg1.ASLP_RATE = 0B00;

    mma8451_write_reg(CTRL_REG1_ADDRESS, ctrl_reg1.data);

//step 2 app note
    //configuro Freefall para los tres ejes, Latched

    ffCfgReg.XEFE=1;
    ffCfgReg.YEFE=1;
    ffCfgReg.ZEFE=1;
    ffCfgReg.OAE=0;
    ffCfgReg.ELE=1;//Latched

    mma8451_write_reg(FF_MT_CFG_ADDRESS, ffCfgReg.data);

//step 3 app note
    //configuro threshold
    //3 cuentas~0.2g

    ffThsReg.THS0=1;
    ffThsReg.THS1=1;
    ffThsReg.THS2=0;
    ffThsReg.THS3=0;
    ffThsReg.THS4=0;
    ffThsReg.THS5=0;
    ffThsReg.THS6=0;
    ffThsReg.DBCNTM=0;

    mma8451_write_reg(FF_MT_THS_ADDRESS, ffThsReg.data);

    //For a freefall algorithm a minimum of 120 ms should be considered for the freefall condition to be met to be considered a linear freefall and not a false trigger.
//step 4 app note
    //Set the debounce counter to eliminate false positive
    //readings for 50Hz sample rate with a requirement of 120 ms timer, assuming Normal Mode.
    //6 cuentas = 120ms, 8 cuentas = 160ms, 10 cuentas = 200ms
    mma8451_write_reg(FF_MT_COUNT_ADDRESS, 0x08);

//step 5 app note

    //Habilito INT para FreeFall
	ctrl_reg4.INT_EN_DRDY = 0;
	ctrl_reg4.INT_EN_FF_MT = 1;
	ctrl_reg4.INT_EN_PULSE = 0;
	ctrl_reg4.INT_EN_LNDPRT = 0;
	ctrl_reg4.INT_EN_TRANS = 0;
	ctrl_reg4.INT_EN_FIFO = 0;
	ctrl_reg4.INT_EN_ASLP = 0;

	mma8451_write_reg(CTRL_REG4_ADDRESS, ctrl_reg4.data);

//step 6 app note
	//Route the Motion/Freefall Interrupt Function to INT1 hardware pin (CTRL_REG5)
	ctrl_reg5.INT_CFG_DRDY = 0;
	ctrl_reg5.INT_CFG_FF_MT = 1;
	ctrl_reg5.INT_CFG_PULSE = 0;
	ctrl_reg5.INT_CFG_LNDPRT = 0;
	ctrl_reg5.INT_CFG_TRANS = 0;
	ctrl_reg5.INT_CFG_FIFO = 0;
	ctrl_reg5.INT_CFG_ASLP = 0;

	mma8451_write_reg(CTRL_REG5_ADDRESS, ctrl_reg5.data);

//step 7 app note
	//Put the device in Active Mode
	//read reg1
	ctrl_reg1.data=mma8451_read_reg(CTRL_REG1_ADDRESS);
	//cambiar a active
	ctrl_reg1.data|= 0x1;
	//write reg1
    mma8451_write_reg(CTRL_REG1_ADDRESS, ctrl_reg1.data);


//step 8 app note
    //Write Interrupt Service Routine Reading the
    //System Interrupt Status and the Motion/Freefall Status

    //vuelvo a habilitar INT por si las deshabilitaron en la caida anterior
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);

}

//Configura el acelerometro para interrumpir por DATA READY, a 12.5Hz
void mma8451_setDR_int(void)
{
    CTRL_REG1_t ctrl_reg1;
    CTRL_REG4_t ctrl_reg4;
    CTRL_REG5_t ctrl_reg5;

    /* Primero desactivo el acelerómetro, luego escribo otros registros*/

	ctrl_reg1.ACTIVE = 0;
	ctrl_reg1.F_READ = 0;
	ctrl_reg1.LNOISE = 1;
	ctrl_reg1.DR = DR_50hz;
	ctrl_reg1.ASLP_RATE = 0B00;

    mma8451_write_reg(CTRL_REG1_ADDRESS, ctrl_reg1.data);

    //Habilito INT para Data Ready
	ctrl_reg4.INT_EN_DRDY = 1;
	ctrl_reg4.INT_EN_FF_MT = 0;
	ctrl_reg4.INT_EN_PULSE = 0;
	ctrl_reg4.INT_EN_LNDPRT = 0;
	ctrl_reg4.INT_EN_TRANS = 0;
	ctrl_reg4.INT_EN_FIFO = 0;
	ctrl_reg4.INT_EN_ASLP = 0;

	mma8451_write_reg(CTRL_REG4_ADDRESS, ctrl_reg4.data);


	ctrl_reg5.INT_CFG_DRDY = 1;
	ctrl_reg5.INT_CFG_FF_MT = 0;
	ctrl_reg5.INT_CFG_PULSE = 0;
	ctrl_reg5.INT_CFG_LNDPRT = 0;
	ctrl_reg5.INT_CFG_TRANS = 0;
	ctrl_reg5.INT_CFG_FIFO = 0;
	ctrl_reg5.INT_CFG_ASLP = 0;

	mma8451_write_reg(CTRL_REG5_ADDRESS, ctrl_reg5.data);

	//Paso acelerometro a ACTIVE
	ctrl_reg1.ACTIVE = 1;
	ctrl_reg1.F_READ = 0;
	ctrl_reg1.LNOISE = 1;
	ctrl_reg1.DR = DR_12p5hz;
	ctrl_reg1.ASLP_RATE = 0B00;

    mma8451_write_reg(CTRL_REG1_ADDRESS, ctrl_reg1.data);

    //vuelvo a habilitar INT por si las deshabilitaron en el reset pasado
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}


