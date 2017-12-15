################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/pthread/pthread_attr.c \
../src/sys/pthread/pthread_attr_init.c \
../src/sys/pthread/pthread_cancel.c \
../src/sys/pthread/pthread_cleanup.c \
../src/sys/pthread/pthread_cond.c \
../src/sys/pthread/pthread_condattr.c \
../src/sys/pthread/pthread_create.c \
../src/sys/pthread/pthread_mutex.c \
../src/sys/pthread/pthread_mutex_init.c \
../src/sys/pthread/pthread_mutexattr.c \
../src/sys/pthread/pthread_mutexattr_init.c \
../src/sys/pthread/pthread_rwlock.c \
../src/sys/pthread/pthread_rwlockattr.c \
../src/sys/pthread/pthread_schedparam.c \
../src/sys/pthread/pthread_self.c 

OBJS += \
./src/sys/pthread/pthread_attr.o \
./src/sys/pthread/pthread_attr_init.o \
./src/sys/pthread/pthread_cancel.o \
./src/sys/pthread/pthread_cleanup.o \
./src/sys/pthread/pthread_cond.o \
./src/sys/pthread/pthread_condattr.o \
./src/sys/pthread/pthread_create.o \
./src/sys/pthread/pthread_mutex.o \
./src/sys/pthread/pthread_mutex_init.o \
./src/sys/pthread/pthread_mutexattr.o \
./src/sys/pthread/pthread_mutexattr_init.o \
./src/sys/pthread/pthread_rwlock.o \
./src/sys/pthread/pthread_rwlockattr.o \
./src/sys/pthread/pthread_schedparam.o \
./src/sys/pthread/pthread_self.o 

C_DEPS += \
./src/sys/pthread/pthread_attr.d \
./src/sys/pthread/pthread_attr_init.d \
./src/sys/pthread/pthread_cancel.d \
./src/sys/pthread/pthread_cleanup.d \
./src/sys/pthread/pthread_cond.d \
./src/sys/pthread/pthread_condattr.d \
./src/sys/pthread/pthread_create.d \
./src/sys/pthread/pthread_mutex.d \
./src/sys/pthread/pthread_mutex_init.d \
./src/sys/pthread/pthread_mutexattr.d \
./src/sys/pthread/pthread_mutexattr_init.d \
./src/sys/pthread/pthread_rwlock.d \
./src/sys/pthread/pthread_rwlockattr.d \
./src/sys/pthread/pthread_schedparam.d \
./src/sys/pthread/pthread_self.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/pthread/%.o: ../src/sys/pthread/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


