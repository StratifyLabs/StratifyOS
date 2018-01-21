cmake_minimum_required (VERSION 3.6)


if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
  message(STATUS "Load Darwin Toolchain for arm")
  set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts CACHE INTERNAL "sos toolchain path")
  set(SOS_TOOLCHAIN_CMAKE_PATH /Users/tgil/git/StratifyOS/cmake CACHE INTERNAL "sos toolchain path")
endif()
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
  message(STATUS "Load Windows Toolchain for arm")
  set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts CACHE INTERNAL "sos toolchain path")
endif()

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-resolve-build-config.cmake)

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  message(STATUS "Load SOS Lib Toolchain for arm: " ${SOS_TOOLCHAIN_CMAKE_PATH})
  set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-toolchain.cmake CACHE INTERNAL "sos toolchain file")
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
  set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake CACHE INTERNAL "sos toolchain file")
endif()

set(SOS_LIB_NAME sos CACHE INTERNAL "sos lib name")
project(${SOS_LIB_NAME} CXX C)

set(SOS_LIB_INCLUDE_DIRECTORIES include CACHE INTERNAL "sos lib include directories")


include( ${CMAKE_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )
