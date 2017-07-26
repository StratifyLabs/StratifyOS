################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/crt/crt.c \
../src/crt/crt_common.c 

CPP_SRCS += \
../src/crt/operator.cpp 

S_UPPER_SRCS += \
../src/crt/crt_symbols.S 

OBJS += \
./src/crt/crt.o \
./src/crt/crt_common.o \
./src/crt/crt_symbols.o \
./src/crt/operator.o 

C_DEPS += \
./src/crt/crt.d \
./src/crt/crt_common.d 

CPP_DEPS += \
./src/crt/operator.d 


# Each subdirectory must supply rules for building sources it contributes
src/crt/%.o: ../src/crt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__crt -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -mlong-calls -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/crt/%.o: ../src/crt/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Assembler'
	arm-none-eabi-g++ -c -march=armv7e-m -mthumb  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/crt/%.o: ../src/crt/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-none-eabi-g++ -D__StratifyOS__ -D__armv7em -D__crt -D__ -I"/Users/tgil/git/StratifyOS/include" -Os -Wall -c -mlong-calls -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


