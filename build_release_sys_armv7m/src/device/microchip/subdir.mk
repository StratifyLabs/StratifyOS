################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/device/microchip/sst25vf.c \
../src/device/microchip/sst25vf_share.c \
../src/device/microchip/sst25vf_ssp.c \
../src/device/microchip/sst25vf_tmr.c 

OBJS += \
./src/device/microchip/sst25vf.o \
./src/device/microchip/sst25vf_share.o \
./src/device/microchip/sst25vf_ssp.o \
./src/device/microchip/sst25vf_tmr.o 

C_DEPS += \
./src/device/microchip/sst25vf.d \
./src/device/microchip/sst25vf_share.d \
./src/device/microchip/sst25vf_ssp.d \
./src/device/microchip/sst25vf_tmr.d 


# Each subdirectory must supply rules for building sources it contributes
src/device/microchip/%.o: ../src/device/microchip/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


