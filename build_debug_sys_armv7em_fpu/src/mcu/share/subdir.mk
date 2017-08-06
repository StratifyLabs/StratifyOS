################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mcu/share/adc.c \
../src/mcu/share/core.c \
../src/mcu/share/dac.c \
../src/mcu/share/dac_dma.c \
../src/mcu/share/eeprom.c \
../src/mcu/share/eint.c \
../src/mcu/share/emc.c \
../src/mcu/share/enet.c \
../src/mcu/share/flash.c \
../src/mcu/share/i2c.c \
../src/mcu/share/i2s.c \
../src/mcu/share/link_transport_slave.c \
../src/mcu/share/mci.c \
../src/mcu/share/mcu.c \
../src/mcu/share/mcu_debug.c \
../src/mcu/share/mcu_sync_io.c \
../src/mcu/share/mem.c \
../src/mcu/share/pio.c \
../src/mcu/share/pwm.c \
../src/mcu/share/qei.c \
../src/mcu/share/rtc.c \
../src/mcu/share/spi.c \
../src/mcu/share/ssp.c \
../src/mcu/share/sys.c \
../src/mcu/share/tmr.c \
../src/mcu/share/uart.c \
../src/mcu/share/usb.c 

OBJS += \
./src/mcu/share/adc.o \
./src/mcu/share/core.o \
./src/mcu/share/dac.o \
./src/mcu/share/dac_dma.o \
./src/mcu/share/eeprom.o \
./src/mcu/share/eint.o \
./src/mcu/share/emc.o \
./src/mcu/share/enet.o \
./src/mcu/share/flash.o \
./src/mcu/share/i2c.o \
./src/mcu/share/i2s.o \
./src/mcu/share/link_transport_slave.o \
./src/mcu/share/mci.o \
./src/mcu/share/mcu.o \
./src/mcu/share/mcu_debug.o \
./src/mcu/share/mcu_sync_io.o \
./src/mcu/share/mem.o \
./src/mcu/share/pio.o \
./src/mcu/share/pwm.o \
./src/mcu/share/qei.o \
./src/mcu/share/rtc.o \
./src/mcu/share/spi.o \
./src/mcu/share/ssp.o \
./src/mcu/share/sys.o \
./src/mcu/share/tmr.o \
./src/mcu/share/uart.o \
./src/mcu/share/usb.o 

C_DEPS += \
./src/mcu/share/adc.d \
./src/mcu/share/core.d \
./src/mcu/share/dac.d \
./src/mcu/share/dac_dma.d \
./src/mcu/share/eeprom.d \
./src/mcu/share/eint.d \
./src/mcu/share/emc.d \
./src/mcu/share/enet.d \
./src/mcu/share/flash.d \
./src/mcu/share/i2c.d \
./src/mcu/share/i2s.d \
./src/mcu/share/link_transport_slave.d \
./src/mcu/share/mci.d \
./src/mcu/share/mcu.d \
./src/mcu/share/mcu_debug.d \
./src/mcu/share/mcu_sync_io.d \
./src/mcu/share/mem.d \
./src/mcu/share/pio.d \
./src/mcu/share/pwm.d \
./src/mcu/share/qei.d \
./src/mcu/share/rtc.d \
./src/mcu/share/spi.d \
./src/mcu/share/ssp.d \
./src/mcu/share/sys.d \
./src/mcu/share/tmr.d \
./src/mcu/share/uart.d \
./src/mcu/share/usb.d 


# Each subdirectory must supply rules for building sources it contributes
src/mcu/share/%.o: ../src/mcu/share/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


