################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/link/link_thread.c \
../src/sys/link/sos_link_transport_usb.c \
../src/sys/link/sos_link_transport_usb_descriptors.c \
../src/sys/link/sos_link_transport_usb_dual_vcp_descriptors.c 

OBJS += \
./src/sys/link/link_thread.o \
./src/sys/link/sos_link_transport_usb.o \
./src/sys/link/sos_link_transport_usb_descriptors.o \
./src/sys/link/sos_link_transport_usb_dual_vcp_descriptors.o 

C_DEPS += \
./src/sys/link/link_thread.d \
./src/sys/link/sos_link_transport_usb.d \
./src/sys/link/sos_link_transport_usb_descriptors.d \
./src/sys/link/sos_link_transport_usb_dual_vcp_descriptors.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/link/%.o: ../src/sys/link/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


