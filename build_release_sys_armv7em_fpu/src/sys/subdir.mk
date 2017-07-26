################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/libgcc.c \
../src/sys/sos_default_thread.c \
../src/sys/sos_led.c \
../src/sys/sos_main.c \
../src/sys/symbols.c \
../src/sys/sys_23_dev.c \
../src/sys/sys_dev.c 

OBJS += \
./src/sys/libgcc.o \
./src/sys/sos_default_thread.o \
./src/sys/sos_led.o \
./src/sys/sos_main.o \
./src/sys/symbols.o \
./src/sys/sys_23_dev.o \
./src/sys/sys_dev.o 

C_DEPS += \
./src/sys/libgcc.d \
./src/sys/sos_default_thread.d \
./src/sys/sos_led.d \
./src/sys/sos_main.d \
./src/sys/symbols.d \
./src/sys/sys_23_dev.d \
./src/sys/sys_dev.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/%.o: ../src/sys/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


