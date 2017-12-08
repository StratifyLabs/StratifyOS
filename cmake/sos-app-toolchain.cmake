

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)


set(TOOLCHAIN_HOST "arm-none-eabi")
set(TOOLCHAIN_DIR "/Applications/StratifyLabs-SDK/Tools/gcc" CACHE INTERNAL "toolchain dir")
set(TOOLCHAIN_BIN_DIR "${TOOLCHAIN_DIR}/bin" CACHE INTERNAL "toolchain bin dir")
set(TOOLCHAIN_INC_DIR "${TOOLCHAIN_DIR}/${TOOLCHAIN_HOST}/include" CACHE INTERNAL "toolchain inc dir")
set(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_DIR}/${TOOLCHAIN_HOST}/lib" CACHE INTERNAL "toolchain lib dir")
set(TOOLCHAIN_LIB_DIR "${TOOLCHAIN_DIR}/${TOOLCHAIN_HOST}/lib" CACHE INTERNAL "toolchain lib dir")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_HOST}-gcc${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_HOST}-g++${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "cxx compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_HOST}-as${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "asm compiler")
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_HOST}-objcopy${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "object copy tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_HOST}-objdump${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "object dump tool")
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_HOST}-size${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "size tool")

set(CMAKE_C_FLAGS_DEBUG "-Og -g -D__StratifyOS__ -mlong-calls -ffunction-sections" CACHE INTERNAL "c compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "cxx compiler flags debug")
set(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "asm compiler flags debug")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker flags debug")

set(CMAKE_C_FLAGS_RELEASE "-mthumb -Os -D__StratifyOS__ -mlong-calls -ffunction-sections -fdata-sections" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE "-mthumb -Os -D__StratifyOS__ -mlong-calls -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit" CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELEASE "-mthumb" CACHE INTERNAL "asm compiler flags release")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "linker flags release")

set(CMAKE_C_FLAGS "-mthumb -Os -D__StratifyOS__ -mlong-calls -ffunction-sections -fdata-sections" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS "-mthumb -Os -D__StratifyOS__ -mlong-calls -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit" CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS "-mthumb" CACHE INTERNAL "asm compiler flags release")


set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)