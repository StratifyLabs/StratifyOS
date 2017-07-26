################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/signal/_kill.c \
../src/sys/signal/_wait.c \
../src/sys/signal/pthread_kill.c \
../src/sys/signal/sig_realtime.c \
../src/sys/signal/signal.c \
../src/sys/signal/signal_handler.c \
../src/sys/signal/sigset.c 

OBJS += \
./src/sys/signal/_kill.o \
./src/sys/signal/_wait.o \
./src/sys/signal/pthread_kill.o \
./src/sys/signal/sig_realtime.o \
./src/sys/signal/signal.o \
./src/sys/signal/signal_handler.o \
./src/sys/signal/sigset.o 

C_DEPS += \
./src/sys/signal/_kill.d \
./src/sys/signal/_wait.d \
./src/sys/signal/pthread_kill.d \
./src/sys/signal/sig_realtime.d \
./src/sys/signal/signal.d \
./src/sys/signal/signal_handler.d \
./src/sys/signal/sigset.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/signal/%.o: ../src/sys/signal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


