

option(BUILD_ARM_ALL "Build All configurations" ON)

option(BUILD_ARM_V7M "Build v7m configuration" OFF)
option(BUILD_ARM_V7EM "Build v7em configuration" OFF)
option(BUILD_ARM_V7EM_F4SH "Build v7em_f4sh configuration" OFF)
option(BUILD_ARM_V7EM_F5SH "Build v7em_f5sh configuration" OFF)
option(BUILD_ARM_V7EM_F5DH "Build v7em_f5dh configuration" OFF)

if(BUILD_ARM_ALL OR BUILD_ARM_V7M)
set(SOS_LIB_ARCH v7m)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

if(BUILD_ARM_ALL OR BUILD_ARM_V7EM)
set(SOS_LIB_ARCH v7em)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

#set(SOS_LIB_ARCH v7em_f4ss)
#include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)


if(BUILD_ARM_ALL OR BUILD_ARM_V7EM_F4SH)
set(SOS_LIB_ARCH v7em_f4sh)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

#set(SOS_LIB_ARCH v7em_f5ss)
#include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)


if(BUILD_ARM_ALL OR BUILD_ARM_V7EM_F5SH)
set(SOS_LIB_ARCH v7em_f5sh)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

#set(SOS_LIB_ARCH v7em_f5ds)
#include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

if(BUILD_ARM_ALL OR BUILD_ARM_V7EM_F5DH)
set(SOS_LIB_ARCH v7em_f5dh)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()
