################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/syscalls/_calloc.c \
../src/sys/syscalls/_realloc.c \
../src/sys/syscalls/_sbrk.c \
../src/sys/syscalls/calloc.c \
../src/sys/syscalls/mallinfo.c \
../src/sys/syscalls/malloc.c \
../src/sys/syscalls/malloc_stats.c \
../src/sys/syscalls/mallocr.c \
../src/sys/syscalls/mlock.c \
../src/sys/syscalls/realloc.c 

OBJS += \
./src/sys/syscalls/_calloc.o \
./src/sys/syscalls/_realloc.o \
./src/sys/syscalls/_sbrk.o \
./src/sys/syscalls/calloc.o \
./src/sys/syscalls/mallinfo.o \
./src/sys/syscalls/malloc.o \
./src/sys/syscalls/malloc_stats.o \
./src/sys/syscalls/mallocr.o \
./src/sys/syscalls/mlock.o \
./src/sys/syscalls/realloc.o 

C_DEPS += \
./src/sys/syscalls/_calloc.d \
./src/sys/syscalls/_realloc.d \
./src/sys/syscalls/_sbrk.d \
./src/sys/syscalls/calloc.d \
./src/sys/syscalls/mallinfo.d \
./src/sys/syscalls/malloc.d \
./src/sys/syscalls/malloc_stats.d \
./src/sys/syscalls/mallocr.d \
./src/sys/syscalls/mlock.d \
./src/sys/syscalls/realloc.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/syscalls/%.o: ../src/sys/syscalls/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D___debug -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


