################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/trace/posix_trace.c \
../src/sys/trace/posix_trace_attr.c \
../src/sys/trace/stratify_trace.c 

OBJS += \
./src/sys/trace/posix_trace.o \
./src/sys/trace/posix_trace_attr.o \
./src/sys/trace/stratify_trace.o 

C_DEPS += \
./src/sys/trace/posix_trace.d \
./src/sys/trace/posix_trace_attr.d \
./src/sys/trace/stratify_trace.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/trace/%.o: ../src/sys/trace/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


