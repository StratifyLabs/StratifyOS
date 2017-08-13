################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/unistd/_close.c \
../src/sys/unistd/_execve.c \
../src/sys/unistd/_exit.c \
../src/sys/unistd/_fcntl.c \
../src/sys/unistd/_fork.c \
../src/sys/unistd/_fstat.c \
../src/sys/unistd/_getpid.c \
../src/sys/unistd/_getppid.c \
../src/sys/unistd/_isatty.c \
../src/sys/unistd/_link.c \
../src/sys/unistd/_lseek.c \
../src/sys/unistd/_open.c \
../src/sys/unistd/_read.c \
../src/sys/unistd/_rename.c \
../src/sys/unistd/_stat.c \
../src/sys/unistd/_symlink.c \
../src/sys/unistd/_unlink.c \
../src/sys/unistd/_write.c \
../src/sys/unistd/access.c \
../src/sys/unistd/chmod.c \
../src/sys/unistd/chown.c \
../src/sys/unistd/ioctl.c \
../src/sys/unistd/lstat.c \
../src/sys/unistd/mkdir.c \
../src/sys/unistd/rmdir.c \
../src/sys/unistd/sleep.c \
../src/sys/unistd/uidgid.c \
../src/sys/unistd/usleep.c 

OBJS += \
./src/sys/unistd/_close.o \
./src/sys/unistd/_execve.o \
./src/sys/unistd/_exit.o \
./src/sys/unistd/_fcntl.o \
./src/sys/unistd/_fork.o \
./src/sys/unistd/_fstat.o \
./src/sys/unistd/_getpid.o \
./src/sys/unistd/_getppid.o \
./src/sys/unistd/_isatty.o \
./src/sys/unistd/_link.o \
./src/sys/unistd/_lseek.o \
./src/sys/unistd/_open.o \
./src/sys/unistd/_read.o \
./src/sys/unistd/_rename.o \
./src/sys/unistd/_stat.o \
./src/sys/unistd/_symlink.o \
./src/sys/unistd/_unlink.o \
./src/sys/unistd/_write.o \
./src/sys/unistd/access.o \
./src/sys/unistd/chmod.o \
./src/sys/unistd/chown.o \
./src/sys/unistd/ioctl.o \
./src/sys/unistd/lstat.o \
./src/sys/unistd/mkdir.o \
./src/sys/unistd/rmdir.o \
./src/sys/unistd/sleep.o \
./src/sys/unistd/uidgid.o \
./src/sys/unistd/usleep.o 

C_DEPS += \
./src/sys/unistd/_close.d \
./src/sys/unistd/_execve.d \
./src/sys/unistd/_exit.d \
./src/sys/unistd/_fcntl.d \
./src/sys/unistd/_fork.d \
./src/sys/unistd/_fstat.d \
./src/sys/unistd/_getpid.d \
./src/sys/unistd/_getppid.d \
./src/sys/unistd/_isatty.d \
./src/sys/unistd/_link.d \
./src/sys/unistd/_lseek.d \
./src/sys/unistd/_open.d \
./src/sys/unistd/_read.d \
./src/sys/unistd/_rename.d \
./src/sys/unistd/_stat.d \
./src/sys/unistd/_symlink.d \
./src/sys/unistd/_unlink.d \
./src/sys/unistd/_write.d \
./src/sys/unistd/access.d \
./src/sys/unistd/chmod.d \
./src/sys/unistd/chown.d \
./src/sys/unistd/ioctl.d \
./src/sys/unistd/lstat.d \
./src/sys/unistd/mkdir.d \
./src/sys/unistd/rmdir.d \
./src/sys/unistd/sleep.d \
./src/sys/unistd/uidgid.d \
./src/sys/unistd/usleep.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/unistd/%.o: ../src/sys/unistd/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__armv7m -D__sys -D__ -I"/Users/tgil/git/StratifyOS/include/posix" -I"/Users/tgil/git/StratifyOS/include" -I"/Users/tgil/git/StratifyOS/src" -I"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/include" -Os -Wall -c -ffunction-sections -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


