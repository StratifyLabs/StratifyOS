cmake_minimum_required (VERSION 3.6)

set(SOS_TOOLCHAIN_CMAKE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-sdk.cmake)

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-toolchain.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
  set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake)
endif()

set(SOS_NAME sos)
project(${SOS_NAME} CXX C ASM)

set(SOS_INCLUDE_DIRECTORIES include)
include( ${CMAKE_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )

if(NOT SOS_SKIP_CMAKE)
	install(DIRECTORY cmake/ DESTINATION cmake)
endif()

add_custom_target(
  format
  COMMAND /usr/local/bin/clang-format
  -i
  ${SOS_SOURCELIST}
)

