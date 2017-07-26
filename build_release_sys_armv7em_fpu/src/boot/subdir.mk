################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/boot/boot_debug.c \
../src/boot/boot_link.c \
../src/boot/boot_link_transport_usb.c \
../src/boot/boot_main.c 

OBJS += \
./src/boot/boot_debug.o \
./src/boot/boot_link.o \
./src/boot/boot_link_transport_usb.o \
./src/boot/boot_main.o 

C_DEPS += \
./src/boot/boot_debug.d \
./src/boot/boot_link.d \
./src/boot/boot_link_transport_usb.d \
./src/boot/boot_main.d 


# Each subdirectory must supply rules for building sources it contributes
src/boot/%.o: ../src/boot/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


