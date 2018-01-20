

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(TOOLCHAIN_DIR)               # <--- Use 'BOOST_DIR', not 'DEFINED ${BOOST_DIR}'
  MESSAGE(STATUS "User provided toolchain directory" ${TOOLCHAIN_DIR})
else()
  if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
    set(TOOLCHAIN_DIR "/Applications/StratifyLabs-SDK/Tools/gcc" CACHE INTERNAL "toolchain dir")
  endif()
  if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
    set(TOOLCHAIN_DIR "C:/StratifyLabs-SDK/Tools/gcc" CACHE INTERNAL "toolchain dir")
  endif()
endif()

if(TOOLCHAIN_HOST)               # <--- Use 'BOOST_DIR', not 'DEFINED ${BOOST_DIR}'
  MESSAGE(STATUS "User provided toolchain directory" ${TOOLCHAIN_HOST})
else()
  set(TOOLCHAIN_HOST "arm-none-eabi")
endif()

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

set(COMMON_C_FLAGS "-mthumb -Os -D__StratifyOS__ -ffunction-sections -fdata-sections" CACHE INTERNAL "common c compiler flags")
set(COMMON_CXX_FLAGS "-fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-use-cxa-atexit" CACHE INTERNAL "common c compiler flags")
set(COMMON_ASM_FLAGS "-mthumb" CACHE INTERNAL "common asm compiler flags")

set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE INTERNAL "verbose make")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
