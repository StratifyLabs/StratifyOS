
if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7M)
set(SOS_ARCH v7m)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM)
set(SOS_ARCH v7em)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

#set(SOS_ARCH v7em_f4ss)
#include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM_F4SH)
set(SOS_ARCH v7em_f4sh)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

#set(SOS_ARCH v7em_f5ss)
#include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)


if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM_F5SH)
set(SOS_ARCH v7em_f5sh)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()

#set(SOS_ARCH v7em_f5ds)
#include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

if(SOS_ARCH_ARM_ALL OR SOS_ARCH_ARM_V7EM_F5DH)
set(SOS_ARCH v7em_f5dh)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
endif()
