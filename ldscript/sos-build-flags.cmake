
set(SOS_BUILD_C_FLAGS "-mthumb -Os -D__StratifyOS__ -ffunction-sections -fdata-sections" CACHE INTERNAL "common c compiler flags")
set(SOS_BUILD_CXX_FLAGS "-fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit" CACHE INTERNAL "common c compiler flags")
set(SOS_BUILD_ASM_FLAGS "-mthumb" CACHE INTERNAL "common asm compiler flags")
set(SOS_BUILD_FLOAT_OPTIONS_ARMV7EM_FPU -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED CACHE INTERNAL "armv7em_fpu build float options")
set(SOS_BUILD_FLOAT_DIR_ARMV7M "." CACHE INTERNAL "armv7m build float dir")
set(SOS_BUILD_FLOAT_DIR_ARMV7EM_FPU "fpu/fpv5-sp-d16" CACHE INTERNAL "armv7m build float dir")

set(BUILD_ARMV7M_C_FLAGS "${BUILD_C_FLAGS} -march=armv7-m" CACHE INTERNAL "armv7m c compiler flags")
set(BUILD_ARMV7M_CXX_FLAGS "${BUILD_ARMV7M_C_FLAGS} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7m cxx compiler flags")
set(BUILD_ARMV7M_ASM_FLAGS "-mthumb -mpu=cortex-m3" CACHE INTERNAL "armv7m asm compiler flags")

set(BUILD_ARMV7EM_FPU_C_FLAGS "${BUILD_C_FLAGS} -march=armv7e-m ${BUILD_FLOAT_OPTIONS}" CACHE INTERNAL "armv7em_fpu c compiler flags")
set(BUILD_ARMV7EM_FPU_CXX_FLAGS "${BUILD_ARMV7EM_FPU_C_FLAGS} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7em_fpu cxx compiler flags")
set(BUILD_ARMV7EM_FPU_ASM_FLAGS "-mthumb -mpu=cortex-m4" CACHE INTERNAL "armv7em_fpu asm compiler flags")

set(BUILD_ARMV7M_C_FLAGS_DEBUG "${BUILD_C_FLAGS} -D___debug -march=armv7-m" CACHE INTERNAL "armv7m debug c compiler flags")
set(BUILD_ARMV7M_CXX_FLAGS_DEBUG "${BUILD_ARMV7M_C_FLAGS_DEBUG} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7m debug cxx compiler flags")
set(BUILD_ARMV7M_ASM_FLAGS_DEBUG "${BUILD_ARMV7M_ASM_FLAGS}" CACHE INTERNAL "armv7m debug asm compiler flags")

set(BUILD_ARMV7EM_FPU_C_FLAGS_DEBUG "${BUILD_C_FLAGS} -D___debug -march=armv7e-m ${BUILD_FLOAT_OPTIONS}" CACHE INTERNAL "armv7em_fpu debug c compiler flags")
set(BUILD_ARMV7EM_FPU_CXX_FLAGS_DEBUG "${BUILD_ARMV7EM_FPU_C_FLAGS_DEBUG} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7em_fpu debug cxx compiler flags")
set(BUILD_ARMV7EM_FPU_ASM_FLAGS_DEBUG "${BUILD_ARMV7EM_FPU_ASM_FLAGS}" CACHE INTERNAL "armv7em_fpu debug asm compiler flags")

set(BUILD_ARMV7M_C_FLAGS_RELEASE "${BUILD_C_FLAGS} -march=armv7-m" CACHE INTERNAL "armv7m release asm c compiler flags")
set(BUILD_ARMV7M_CXX_FLAGS_RELEASE "${BUILD_ARMV7M_C_FLAGS_RELEASE} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7m release asm cxx compiler flags")
set(BUILD_ARMV7M_ASM_FLAGS_RELEASE "${BUILD_ARMV7M_ASM_FLAGS}" CACHE INTERNAL "armv7m release asm compiler flags")

set(BUILD_ARMV7EM_FPU_C_FLAGS_RELEASE "${BUILD_C_FLAGS} -march=armv7e-m ${BUILD_FLOAT_OPTIONS}" CACHE INTERNAL "armv7em_fpu release c compiler flags")
set(BUILD_ARMV7EM_FPU_CXX_FLAGS_RELEASE "${BUILD_ARMV7EM_FPU_C_FLAGS_RELEASE} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7em_fpu release cxx compiler flags")
set(BUILD_ARMV7EM_FPU_ASM_FLAGS_RELEASE "${BUILD_ARMV7EM_FPU_ASM_FLAGS}" CACHE INTERNAL "armv7em_fpu release asm compiler flags")

set(BUILD_ARMV7M_C_FLAGS_APP "${BUILD_C_FLAGS} -mlong-calls -march=armv7-m" CACHE INTERNAL "armv7m app c compiler flags")
set(BUILD_ARMV7M_CXX_FLAGS_APP "${BUILD_ARMV7M_C_FLAGS_APP} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7m app cxx compiler flags")
set(BUILD_ARMV7M_ASM_FLAGS_APP "${BUILD_ARMV7M_ASM_FLAGS}" CACHE INTERNAL "armv7m app asm compiler flags")

set(BUILD_ARMV7EM_FPU_C_FLAGS_APP "${BUILD_C_FLAGS} -mlong-calls -march=armv7e-m ${BUILD_FLOAT_OPTIONS}" CACHE INTERNAL "armv7em_fpu app c compiler flags")
set(BUILD_ARMV7EM_FPU_CXX_FLAGS_APP "${BUILD_ARMV7EM_FPU_C_FLAGS_APP} ${BUILD_CXX_FLAGS}" CACHE INTERNAL "armv7em_fpu app cxx compiler flags")
set(BUILD_ARMV7EM_FPU_ASM_FLAGS_APP "${BUILD_ARMV7EM_FPU_ASM_FLAGS}" CACHE INTERNAL "armv7em_fpu app asm compiler flags")
