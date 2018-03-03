
set(SOS_BUILD_C_FLAGS "-Wall -mthumb -D__StratifyOS__ -ffunction-sections -fdata-sections -fomit-frame-pointer")
set(SOS_BUILD_CXX_FLAGS "-fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit")
set(SOS_BUILD_ASM_FLAGS "-mthumb")
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPU -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED)
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPUV5 -mfloat-abi=hard -mfpu=fpv5-d16 -U__SOFTFP__ -D__FPU_USED)

set(SOS_BUILD_FLOAT_DIR_ARMV7M ".")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPU "fpv4-sp/hard")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPUV5 "fpv5/hard")
set(SOS_BUILD_INSTALL_DIR_ARMV7M "thumb/v7-m")
set(SOS_BUILD_INSTALL_DIR_ARMV7EM_FPU "thumb/v7e-m")
set(SOS_BUILD_INSTALL_DIR_ARMV7EM_FPUV5 "thumb/v7e-m")
