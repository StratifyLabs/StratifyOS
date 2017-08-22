################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mcu/lpc/adc_dev.c \
../src/mcu/lpc/core_startup.c \
../src/mcu/lpc/crc_dev.c \
../src/mcu/lpc/dac_dma_dev.c \
../src/mcu/lpc/dma_dev.c \
../src/mcu/lpc/enet_dev.c \
../src/mcu/lpc/flash_dev.c \
../src/mcu/lpc/i2c_dev.c \
../src/mcu/lpc/i2s_dev.c \
../src/mcu/lpc/lpc_flash.c \
../src/mcu/lpc/mcu_dev.c \
../src/mcu/lpc/mem_dev.c \
../src/mcu/lpc/mpu_dev.c \
../src/mcu/lpc/pwm_dev.c \
../src/mcu/lpc/qei_dev.c \
../src/mcu/lpc/rtc_dev.c \
../src/mcu/lpc/spi_dev.c \
../src/mcu/lpc/ssp_dev.c \
../src/mcu/lpc/tmr_dev.c \
../src/mcu/lpc/uart_dev.c \
../src/mcu/lpc/wdt.c 

OBJS += \
./src/mcu/lpc/adc_dev.o \
./src/mcu/lpc/core_startup.o \
./src/mcu/lpc/crc_dev.o \
./src/mcu/lpc/dac_dma_dev.o \
./src/mcu/lpc/dma_dev.o \
./src/mcu/lpc/enet_dev.o \
./src/mcu/lpc/flash_dev.o \
./src/mcu/lpc/i2c_dev.o \
./src/mcu/lpc/i2s_dev.o \
./src/mcu/lpc/lpc_flash.o \
./src/mcu/lpc/mcu_dev.o \
./src/mcu/lpc/mem_dev.o \
./src/mcu/lpc/mpu_dev.o \
./src/mcu/lpc/pwm_dev.o \
./src/mcu/lpc/qei_dev.o \
./src/mcu/lpc/rtc_dev.o \
./src/mcu/lpc/spi_dev.o \
./src/mcu/lpc/ssp_dev.o \
./src/mcu/lpc/tmr_dev.o \
./src/mcu/lpc/uart_dev.o \
./src/mcu/lpc/wdt.o 

C_DEPS += \
./src/mcu/lpc/adc_dev.d \
./src/mcu/lpc/core_startup.d \
./src/mcu/lpc/crc_dev.d \
./src/mcu/lpc/dac_dma_dev.d \
./src/mcu/lpc/dma_dev.d \
./src/mcu/lpc/enet_dev.d \
./src/mcu/lpc/flash_dev.d \
./src/mcu/lpc/i2c_dev.d \
./src/mcu/lpc/i2s_dev.d \
./src/mcu/lpc/lpc_flash.d \
./src/mcu/lpc/mcu_dev.d \
./src/mcu/lpc/mem_dev.d \
./src/mcu/lpc/mpu_dev.d \
./src/mcu/lpc/pwm_dev.d \
./src/mcu/lpc/qei_dev.d \
./src/mcu/lpc/rtc_dev.d \
./src/mcu/lpc/spi_dev.d \
./src/mcu/lpc/ssp_dev.d \
./src/mcu/lpc/tmr_dev.d \
./src/mcu/lpc/uart_dev.d \
./src/mcu/lpc/wdt.d 


# Each subdirectory must supply rules for building sources it contributes
src/mcu/lpc/%.o: ../src/mcu/lpc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__lpc43xx -D__mcu_lpc43xx -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include/mcu/arch/cmsis" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -I"/Users/tgil/git/StratifyOS/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


