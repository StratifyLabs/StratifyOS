
# the name of the target operating system

message(STATUS "Set GCC toolchain to i686-w64-mingw32")
if(TOOLCHAIN_HOST)
    message(STATUS "Toolchain host " ${TOOLCHAIN_HOST})
else()
    set(TOOLCHAIN_HOST i686-w64-mingw32 CACHE INTERNAL "" FORCE)
endif()
if(TOOLCHAIN_DIR)
    message(STATUS "Toolchain directory " ${TOOLCHAIN_DIR})
else()
    set(TOOLCHAIN_DIR C:/StratifyLabs-SDK/Tools/mingw530_32)
endif()
set(TOOLCHAIN_EXEC_SUFFIX .exe)

set(SOS_SDK_LIB_DIR ${SOS_TOOLCHAIN_CMAKE_PATH}/../../lib)


set(SDK_DIR C:/StratifyLabs-SDK/Tools/gcc)

include(${CMAKE_CURRENT_LIST_DIR}/sos-gcc-toolchain.cmake)

set(TOOLCHAIN_C_FLAGS "-D__win32 -mno-ms-bitfields" CACHE INTERNAL "MINGW C FLAGS")
set(TOOLCHAIN_CXX_FLAGS "${TOOLCHAIN_C_FLAGS} -std=c++11" CACHE INTERNAL "MINGW CXX FLAGS")

set(CMAKE_INSTALL_PREFIX ${SDK_DIR})
include_directories(SYSTEM ${SDK_DIR}/include)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS ${TOOLCHAIN_C_FLAGS} CACHE INTERNAL "MINGW C FLAGS")
set(CMAKE_CXX_FLAGS ${TOOLCHAIN_CXX_FLAGS} CACHE INTERNAL "MINGW CXX FLAGS")
