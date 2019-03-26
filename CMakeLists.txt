cmake_minimum_required (VERSION 3.6)

set(SOS_TOOLCHAIN_CMAKE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-resolve-build-config.cmake)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)


if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  message(STATUS "Load SOS Lib Toolchain for arm: " ${SOS_TOOLCHAIN_CMAKE_PATH})
  set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-toolchain.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
  set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake)
endif()

set(SOS_NAME sos)
project(${SOS_NAME} CXX C ASM)

set(SOS_INCLUDE_DIRECTORIES include)
include( ${CMAKE_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )
