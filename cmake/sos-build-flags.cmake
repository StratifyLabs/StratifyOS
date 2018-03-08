
set(SOS_BUILD_C_FLAGS "-Wall -mthumb -D__StratifyOS__ -ffunction-sections -fdata-sections -fomit-frame-pointer")
set(SOS_BUILD_CXX_FLAGS "-fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit")
set(SOS_BUILD_ASM_FLAGS "-mthumb")
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPU -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED)
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPUV5 -mfloat-abi=hard -mfpu=fpv5-d16 -U__SOFTFP__ -D__FPU_USED)


# v7m or v7em for M3 or M4 ISA
# fxyz x = 4 or 5 for FPU version, y = s or d for single or double, z = s or h for soft or hard
#CM3: v7m (no FPU)
#CM4: v7em, v7em_f4ss, v7em_f4sh
#CM7: v7em, v7em_f5ss, v7em_f5sh, v7em_f5ds, v7em_f5dh
#These are values for SOS_LIB_ARCH, SOS_BSP_ARCH and SOS_APP_ARCH


set(SOS_BUILD_FLOAT_DIR_ARMV7M ".")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPU "fpv4-sp/hard")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPUV5 "fpv5/hard")

set(SOS_BUILD_INSTALL_DIR_ARMV7M "thumb/v7-m")
set(SOS_BUILD_INSTALL_DIR_ARMV7EM_FPU "thumb/v7e-m")
set(SOS_BUILD_INSTALL_DIR_ARMV7EM_FPUV5 "thumb/v7e-m")
