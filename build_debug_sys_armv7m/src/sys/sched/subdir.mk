################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/sched/sched.c \
../src/sys/sched/sched_debug.c \
../src/sys/sched/sched_fault.c \
../src/sys/sched/sched_fault_handler.c \
../src/sys/sched/sched_init.c \
../src/sys/sched/sched_posix.c \
../src/sys/sched/sched_priv.c \
../src/sys/sched/sched_process.c \
../src/sys/sched/sched_thread.c \
../src/sys/sched/sched_timing.c 

OBJS += \
./src/sys/sched/sched.o \
./src/sys/sched/sched_debug.o \
./src/sys/sched/sched_fault.o \
./src/sys/sched/sched_fault_handler.o \
./src/sys/sched/sched_init.o \
./src/sys/sched/sched_posix.o \
./src/sys/sched/sched_priv.o \
./src/sys/sched/sched_process.o \
./src/sys/sched/sched_thread.o \
./src/sys/sched/sched_timing.o 

C_DEPS += \
./src/sys/sched/sched.d \
./src/sys/sched/sched_debug.d \
./src/sys/sched/sched_fault.d \
./src/sys/sched/sched_fault_handler.d \
./src/sys/sched/sched_init.d \
./src/sys/sched/sched_posix.d \
./src/sys/sched/sched_priv.d \
./src/sys/sched/sched_process.d \
./src/sys/sched/sched_thread.d \
./src/sys/sched/sched_timing.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/sched/%.o: ../src/sys/sched/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


