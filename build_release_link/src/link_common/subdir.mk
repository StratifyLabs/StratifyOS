################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/link_common/link_transport.c 

OBJS += \
./src/link_common/link_transport.o 

C_DEPS += \
./src/link_common/link_transport.d 


# Each subdirectory must supply rules for building sources it contributes
src/link_common/%.o: ../src/link_common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	clang -m64 -arch x86_64 -mmacosx-version-min=10.9 -D__StratifyOS__ -D__macosx -D__link -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/include" -O3 -Wall -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


