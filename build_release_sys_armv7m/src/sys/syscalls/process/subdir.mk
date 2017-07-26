################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/syscalls/process/_system.c \
../src/sys/syscalls/process/install.c \
../src/sys/syscalls/process/launch.c \
../src/sys/syscalls/process/process_start.c 

OBJS += \
./src/sys/syscalls/process/_system.o \
./src/sys/syscalls/process/install.o \
./src/sys/syscalls/process/launch.o \
./src/sys/syscalls/process/process_start.o 

C_DEPS += \
./src/sys/syscalls/process/_system.d \
./src/sys/syscalls/process/install.d \
./src/sys/syscalls/process/launch.d \
./src/sys/syscalls/process/process_start.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/syscalls/process/%.o: ../src/sys/syscalls/process/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


