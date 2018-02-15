
set(SOS_BUILD_C_FLAGS "-mthumb -Os -D__StratifyOS__ -ffunction-sections -fdata-sections" CACHE INTERNAL "common c compiler flags")
set(SOS_BUILD_CXX_FLAGS "-fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit" CACHE INTERNAL "common c compiler flags")
set(SOS_BUILD_ASM_FLAGS "-mthumb" CACHE INTERNAL "common asm compiler flags")
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPU -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED CACHE INTERNAL "armv7em_fpu build float options")

set(SOS_BUILD_FLOAT_DIR_ARMV7M "." CACHE INTERNAL "armv7m build float dir")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPU "fpv4-sp/hard" CACHE INTERNAL "armv7m build float dir")
set(SOS_BUILD_INSTALL_DIR_ARMV7M "thumb/v7-m" CACHE INTERNAL "armv7m install dir")
set(SOS_BUILD_INSTALL_DIR_ARMV7EM_FPU "thumb/v7e-m" CACHE INTERNAL "armv7m build float dir")
