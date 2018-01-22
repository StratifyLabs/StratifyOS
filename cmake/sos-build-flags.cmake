
set(SOS_BUILD_C_FLAGS "-mthumb -Os -D__StratifyOS__ -ffunction-sections -fdata-sections" CACHE INTERNAL "common c compiler flags")
set(SOS_BUILD_CXX_FLAGS "-fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit" CACHE INTERNAL "common c compiler flags")
set(SOS_BUILD_ASM_FLAGS "-mthumb" CACHE INTERNAL "common asm compiler flags")
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPU -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED CACHE INTERNAL "armv7em_fpu build float options")
set(SOS_BUILD_FLOAT_DIR_ARMV7M "." CACHE INTERNAL "armv7m build float dir")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPU "fpu/fpv5-sp-d16" CACHE INTERNAL "armv7m build float dir")
