################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/SD2_I2C.c \
../source/SD2_board.c \
../source/display.c \
../source/enviarAccContinuo.c \
../source/font5x7.c \
../source/main.c \
../source/mefDisplay.c \
../source/mefRecTrama.c \
../source/mma8451.c \
../source/mtb.c \
../source/oled.c \
../source/procTrama.c \
../source/ringBuffer.c \
../source/semihost_hardfault.c \
../source/uart_ringBufer.c 

C_DEPS += \
./source/SD2_I2C.d \
./source/SD2_board.d \
./source/display.d \
./source/enviarAccContinuo.d \
./source/font5x7.d \
./source/main.d \
./source/mefDisplay.d \
./source/mefRecTrama.d \
./source/mma8451.d \
./source/mtb.d \
./source/oled.d \
./source/procTrama.d \
./source/ringBuffer.d \
./source/semihost_hardfault.d \
./source/uart_ringBufer.d 

OBJS += \
./source/SD2_I2C.o \
./source/SD2_board.o \
./source/display.o \
./source/enviarAccContinuo.o \
./source/font5x7.o \
./source/main.o \
./source/mefDisplay.o \
./source/mefRecTrama.o \
./source/mma8451.o \
./source/mtb.o \
./source/oled.o \
./source/procTrama.o \
./source/ringBuffer.o \
./source/semihost_hardfault.o \
./source/uart_ringBufer.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MKL46Z256VLH4_cm0plus -DCPU_MKL46Z256VLH4 -DCPU_MKL46Z256VLL4 -DFRDM_KL46Z -DFREEDOM -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I../board -I../source -I../ -I../drivers -I../CMSIS -I../startup -I../utilities -I../board/src -O0 -fno-common -g -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/SD2_I2C.d ./source/SD2_I2C.o ./source/SD2_board.d ./source/SD2_board.o ./source/display.d ./source/display.o ./source/enviarAccContinuo.d ./source/enviarAccContinuo.o ./source/font5x7.d ./source/font5x7.o ./source/main.d ./source/main.o ./source/mefDisplay.d ./source/mefDisplay.o ./source/mefRecTrama.d ./source/mefRecTrama.o ./source/mma8451.d ./source/mma8451.o ./source/mtb.d ./source/mtb.o ./source/oled.d ./source/oled.o ./source/procTrama.d ./source/procTrama.o ./source/ringBuffer.d ./source/ringBuffer.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o ./source/uart_ringBufer.d ./source/uart_ringBufer.o

.PHONY: clean-source

