cmake_minimum_required (VERSION 3.12)

if(NOT SOS_SDK_PATH)
	set(SOS_SDK_PATH $ENV{SOS_SDK_PATH})
endif()

set(CMAKE_MODULE_PATH ${SOS_SDK_PATH}/cmake)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_MODULE_PATH}/sos-toolchain.cmake)
include(sos-sdk)

set(SOS_NAME sos)
project(StratifyOS CXX C ASM)

set(SOS_INCLUDE_DIRECTORIES include)
include( ${CMAKE_CURRENT_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/StratifyOS.cmake
	DESTINATION ${SOS_SDK_PATH}/cmake/targets)

add_custom_target(
  format
  COMMAND /usr/local/bin/clang-format
  -i
  ${SOS_SOURCELIST}
)

