################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/sffs/sffs.c \
../src/sys/sffs/sffs_block.c \
../src/sys/sffs/sffs_dev.c \
../src/sys/sffs/sffs_dir.c \
../src/sys/sffs/sffs_file.c \
../src/sys/sffs/sffs_filelist.c \
../src/sys/sffs/sffs_list.c \
../src/sys/sffs/sffs_scratch.c \
../src/sys/sffs/sffs_serialno.c \
../src/sys/sffs/sffs_tp.c 

OBJS += \
./src/sys/sffs/sffs.o \
./src/sys/sffs/sffs_block.o \
./src/sys/sffs/sffs_dev.o \
./src/sys/sffs/sffs_dir.o \
./src/sys/sffs/sffs_file.o \
./src/sys/sffs/sffs_filelist.o \
./src/sys/sffs/sffs_list.o \
./src/sys/sffs/sffs_scratch.o \
./src/sys/sffs/sffs_serialno.o \
./src/sys/sffs/sffs_tp.o 

C_DEPS += \
./src/sys/sffs/sffs.d \
./src/sys/sffs/sffs_block.d \
./src/sys/sffs/sffs_dev.d \
./src/sys/sffs/sffs_dir.d \
./src/sys/sffs/sffs_file.d \
./src/sys/sffs/sffs_filelist.d \
./src/sys/sffs/sffs_list.d \
./src/sys/sffs/sffs_scratch.d \
./src/sys/sffs/sffs_serialno.d \
./src/sys/sffs/sffs_tp.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/sffs/%.o: ../src/sys/sffs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7em -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


