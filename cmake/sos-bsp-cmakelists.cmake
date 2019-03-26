#Copy this file to the OS package project folder as CMakeLists.txt
cmake_minimum_required (VERSION 3.6)

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
	set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
	set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()

# Use the BSP toolchain
set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-toolchain.cmake)

# Include standard variable values
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

# Set the project name
get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${SOS_NAME} CXX C)

# Add sources to the kernel
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)
list(APPEND SOS_KERNEL_SOURCELIST ${SOURCES} ${CMAKE_SOURCE_DIR}/README.md)

# Add sources to the bootloader
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/boot)
add_subdirectory(boot)
list(APPEND SOS_BOOT_SOURCELIST ${SOURCES})

# Set these according to the device you are building for
set(SOS_ARCH v7em_f5sh)
set(SOS_DEVICE stm32f746zg)
set(SOS_DEVICE_FAMILY stm32f746xx)
set(SOS_HARDWARD_ID 0x00000013)
set(SOS_LIBRARIES son_kernel)
set(SOS_INCLUDE_DIRECTORIES ${TOOLCHAIN_INC_DIR}/mcu/arch/stm32/cmsis)

set(SOS_KERNEL_START_ADDRESS 0x08080000)
set(SOS_BOOT_START_ADDRESS 0x08000000)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-std.cmake)
