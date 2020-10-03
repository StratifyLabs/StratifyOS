cmake_minimum_required (VERSION 3.12)

if(DEFINED ENV{SOS_SDK_PATH})
	set(SOS_SDK_PATH $ENV{SOS_SDK_PATH})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_MODULE_PATH}/sos-toolchain.cmake)
include(sos-sdk)

set(SOS_NAME sos)
project(${SOS_NAME} CXX C ASM)

set(SOS_INCLUDE_DIRECTORIES include)
include( ${CMAKE_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )

add_custom_target(
  format
  COMMAND /usr/local/bin/clang-format
  -i
  ${SOS_SOURCELIST}
)

