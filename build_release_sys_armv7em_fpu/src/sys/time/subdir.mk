################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/time/_gettimeofday.c \
../src/sys/time/_itimer.c \
../src/sys/time/_settimeofday.c \
../src/sys/time/_times.c \
../src/sys/time/clocks.c \
../src/sys/time/hibernate.c 

OBJS += \
./src/sys/time/_gettimeofday.o \
./src/sys/time/_itimer.o \
./src/sys/time/_settimeofday.o \
./src/sys/time/_times.o \
./src/sys/time/clocks.o \
./src/sys/time/hibernate.o 

C_DEPS += \
./src/sys/time/_gettimeofday.d \
./src/sys/time/_itimer.d \
./src/sys/time/_settimeofday.d \
./src/sys/time/_times.d \
./src/sys/time/clocks.d \
./src/sys/time/hibernate.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/time/%.o: ../src/sys/time/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


