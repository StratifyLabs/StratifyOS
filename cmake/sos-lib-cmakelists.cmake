#This is a template for a Stratiyf OS Library. Copy the contents to the project folder as CMakeLists.txt and update variable accordingly
cmake_minimum_required (VERSION 3.6)

#Grabs the SOS CMAKE SDK from the default install location -- update as needed
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
	set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
	set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-toolchain.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake)
endif()

set(SOS_NAME api)
project(${SOS_NAME} CXX C)

install(DIRECTORY include DESTINATION include)
set(SOS_INCLUDE_DIRECTORIES include)

include( ${CMAKE_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )
