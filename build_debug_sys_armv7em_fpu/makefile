################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/sys/unistd/subdir.mk
-include src/sys/trace/subdir.mk
-include src/sys/time/subdir.mk
-include src/sys/termios/subdir.mk
-include src/sys/sysfs/subdir.mk
-include src/sys/syscalls/process/subdir.mk
-include src/sys/syscalls/subdir.mk
-include src/sys/socket/subdir.mk
-include src/sys/signal/subdir.mk
-include src/sys/sffs/subdir.mk
-include src/sys/semaphore/subdir.mk
-include src/sys/sched/subdir.mk
-include src/sys/pwd/subdir.mk
-include src/sys/pthread/subdir.mk
-include src/sys/mqueue/subdir.mk
-include src/sys/link/subdir.mk
-include src/sys/grp/subdir.mk
-include src/sys/dirent/subdir.mk
-include src/sys/crt/subdir.mk
-include src/sys/aio/subdir.mk
-include src/sys/subdir.mk
-include src/mcu/share/subdir.mk
-include src/link_common/subdir.mk
-include src/device/microchip/subdir.mk
-include src/device/subdir.mk
-include src/cortex_m/subdir.mk
-include src/boot/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libStratifyOS.a

# Tool invocations
libStratifyOS.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Archiver'
	arm-none-eabi-ar -r  stratify_sys_debug.a $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(ARCHIVES)$(C_DEPS)$(CPP_DEPS) libStratifyOS.a
	-@echo ' '

post-build:
	-/Users/tgil/git/StratifyOS/script/post-process.sh /Applications/StratifyLabs-SDK/Tools/gcc arm-none-eabi stratify sys armv7e-m fpu/fpv5-sp-d16 _debug
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
