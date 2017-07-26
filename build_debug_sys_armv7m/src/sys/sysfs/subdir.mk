################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/sysfs/appfs.c \
../src/sys/sysfs/appfs_ram.c \
../src/sys/sysfs/appfs_util.c \
../src/sys/sysfs/devfs.c \
../src/sys/sysfs/rootfs.c \
../src/sys/sysfs/sysfs.c 

OBJS += \
./src/sys/sysfs/appfs.o \
./src/sys/sysfs/appfs_ram.o \
./src/sys/sysfs/appfs_util.o \
./src/sys/sysfs/devfs.o \
./src/sys/sysfs/rootfs.o \
./src/sys/sysfs/sysfs.o 

C_DEPS += \
./src/sys/sysfs/appfs.d \
./src/sys/sysfs/appfs_ram.d \
./src/sys/sysfs/appfs_util.d \
./src/sys/sysfs/devfs.d \
./src/sys/sysfs/rootfs.d \
./src/sys/sysfs/sysfs.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/sysfs/%.o: ../src/sys/sysfs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


