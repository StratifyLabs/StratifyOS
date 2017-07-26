################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/link/link.c \
../src/link/link_bootloader.c \
../src/link/link_debug.c \
../src/link/link_dir.c \
../src/link/link_file.c \
../src/link/link_phy.c \
../src/link/link_posix_trace.c \
../src/link/link_process.c \
../src/link/link_protocol_master.c \
../src/link/link_stdio.c \
../src/link/link_sys_attr.c \
../src/link/link_time.c 

OBJS += \
./src/link/link.o \
./src/link/link_bootloader.o \
./src/link/link_debug.o \
./src/link/link_dir.o \
./src/link/link_file.o \
./src/link/link_phy.o \
./src/link/link_posix_trace.o \
./src/link/link_process.o \
./src/link/link_protocol_master.o \
./src/link/link_stdio.o \
./src/link/link_sys_attr.o \
./src/link/link_time.o 

C_DEPS += \
./src/link/link.d \
./src/link/link_bootloader.d \
./src/link/link_debug.d \
./src/link/link_dir.d \
./src/link/link_file.d \
./src/link/link_phy.d \
./src/link/link_posix_trace.d \
./src/link/link_process.d \
./src/link/link_protocol_master.d \
./src/link/link_stdio.d \
./src/link/link_sys_attr.d \
./src/link/link_time.d 


# Each subdirectory must supply rules for building sources it contributes
src/link/%.o: ../src/link/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	clang -m64 -arch x86_64 -mmacosx-version-min=10.9 -D__StratifyOS__ -D__macosx -D__link -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/include" -O3 -Wall -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


