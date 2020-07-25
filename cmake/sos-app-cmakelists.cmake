#Copy this file to the application project folder as CMakeLists.txt
cmake_minimum_required (VERSION 3.6)

#Grabs the SOS CMAKE SDK from the default install location -- update as needed
# Grab the SDK location
if(NOT DEFINED SOS_SDK_PATH)
	if(DEFINED ENV{SOS_SDK_PATH})
		set(SOS_SDK_PATH $ENV{SOS_SDK_PATH})
	elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
		message(WARNING "default SDK locations are deprecated")
		set(SOS_SDK_PATH /Applications/StratifyLabs-SDK)
	elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
		message(WARNING "default SDK locations are deprecated")
		set(SOS_SDK_PATH C:/StratifyLabs-SDK)
	elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux" )
		message(WARNING "default SDK locations are deprecated")
		set(SOS_SDK_PATH /StratifyLabs-SDK)
	endif()
endif()
if(NOT DEFINED SOS_SDK_PATH)
	message(FATAL_ERROR "Please set environment variable SOS_SDK_PATH to location of the StratifyLabs-SDK directory")
endif()
set(SOS_TOOLCHAIN_CMAKE_PATH ${SOS_SDK_PATH}/Tools/gcc/arm-none-eabi/cmake)

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-app-toolchain.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake)
endif()

# This will set the default RAM used by the application
set(SOS_RAM_SIZE 4096)

#Add sources to the project
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)
list(APPEND SOS_SOURCELIST ${SOURCES})

get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${SOS_NAME} CXX C)

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-app-std.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
	set(SOS_ARCH link)
	set(SOS_OPTION "")
	set(SOS_CONFIG release)
	include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-app.cmake)
endif()
