################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cortex_m/cortexm.c \
../src/cortex_m/fault.c \
../src/cortex_m/mpu.c \
../src/cortex_m/task.c \
../src/cortex_m/task_mpu.c \
../src/cortex_m/task_process.c \
../src/cortex_m/task_single.c 

OBJS += \
./src/cortex_m/cortexm.o \
./src/cortex_m/fault.o \
./src/cortex_m/mpu.o \
./src/cortex_m/task.o \
./src/cortex_m/task_mpu.o \
./src/cortex_m/task_process.o \
./src/cortex_m/task_single.o 

C_DEPS += \
./src/cortex_m/cortexm.d \
./src/cortex_m/fault.d \
./src/cortex_m/mpu.d \
./src/cortex_m/task.d \
./src/cortex_m/task_mpu.d \
./src/cortex_m/task_process.d \
./src/cortex_m/task_single.d 


# Each subdirectory must supply rules for building sources it contributes
src/cortex_m/%.o: ../src/cortex_m/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


