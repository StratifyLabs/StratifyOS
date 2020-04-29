---
date: "2019-06-27"
layout: post
title: Understanding Stratify OS cmake Projects
katex: true
categories: Guides
slug: Guide-CMake
menu:
  sidebar:
    name: CMake
    parent: Guides
---

Stratify OS includes a suite of cmake scripts that can be used to create:

- Applications
- Libraries
- OS Packages

# Building Applications

The HelloWorld project has a CMakeLists.txt file that looks like the following. Additional comments have been added for clarity.

```
cmake_minimum_required (VERSION 3.6) # required by cmake so it knows the version

# this checks for mac or windows and sets up the location of the StratifyOS SDK
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
	set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
elseif( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
	set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()

# Don't try to override any variables above this
# This includes the variables listed in the docs below
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

# SOS_* variables can start to be overriden here

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-app-toolchain.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake)
endif()

# This will set the default RAM used by the application
set(SOS_RAM_SIZE 4096)

# This will grab the CMakeLists.txt file from the src directory
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)

# Now takes the sources from the src directory and put it in a list for the SDK
list(APPEND SOS_SOURCELIST ${SOURCES})

# SOS_NAME must be set to the project directory name for applications
get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${SOS_NAME} CXX C)

# SOS_BUILD_CONFIG is set by sos-variables and looks at the suffix of the build diretory
if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	# this means the build directory ends in _arm (usually cmake_arm)
	#This will cross compile for all supported ARM architetures
	include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-app-std.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
	# this means the build directory ends in _link_ (usually cmake_link)
	#This will compile natively. If the program uses hardware, it can "link" to a board over USB
	set(SOS_ARCH link)
	set(SOS_CONFIG release)
	include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-app.cmake)
endif()
```

# Building Libraries

Builing libraries is a bit more advanced especially if you want them to use them with both arm and link (native) programs. The CMakeLists.txt file below is for the sgfx library. You also need to provide an `arm.cmake` file and a `link.cmake` file. The easiest way to get started is [look at the source](https://github.com/StratifyLabs/sgfx).

```
cmake_minimum_required (VERSION 3.6)

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
  set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
  set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()
set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-toolchain.cmake)
get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-toolchain.cmake)
elseif( ${SOS_BUILD_CONFIG} STREQUAL link )
	set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/link-toolchain.cmake)
endif()

set(SOS_NAME sgfx)
project(${SOS_NAME} CXX C)

install(DIRECTORY include/ DESTINATION include/sapi)
set(SOS_INCLUDE_DIRECTORIES include)
include( ${CMAKE_SOURCE_DIR}/${SOS_BUILD_CONFIG}.cmake )
```

# Building OS Packages

The mbedLPC1768 OS package builds configurations that can be used with both the UART (mbed serial port) or the native USB. There are many other examples on the [Stratify Labs Github account](https://github.com/StratifyLabs).

```
cmake_minimum_required (VERSION 3.6)

if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin" )
  set(SOS_TOOLCHAIN_CMAKE_PATH /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/cmake)
endif()
if( ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows" )
  set(SOS_TOOLCHAIN_CMAKE_PATH C:/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts)
endif()

include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-variables.cmake)

set(CMAKE_TOOLCHAIN_FILE ${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-toolchain.cmake)
get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${SOS_NAME} CXX C)

#Add sources to the project
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)
list(APPEND SOS_KERNEL_SOURCELIST ${SOURCES})

#Change to boot sources
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/boot)
add_subdirectory(boot)
list(APPEND BOOT_SOURCELIST ${SOURCES})

set(SOS_ARCH v7m)
set(SOS_DEVICE lpc1768)
set(SOS_DEVICE_FAMILY lpc17xx)
set(SOS_HARDWARD_ID 0x00000003)

set(SOS_KERNEL_START_ADDRESS 0x40000)
set(SOS_BOOT_START_ADDRESS 0x0)
set(SOS_BOOT_SOURCELIST ${BOOT_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp-std.cmake)

set(SOS_DEFINITIONS __UART=1)
set(SOS_START_ADDRESS 0x40000)
set(SOS_SOURCELIST ${SOS_KERNEL_SOURCELIST})
set(SOS_CONFIG uart)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-bsp.cmake)
```

# Variable Reference

The Stratify OS SDK includes several cmake variables that can be assigned to change how the project builds.

## SOS_NAME

Applies to Applications (A), Libraries (L), and OS Packages (O).

This variables specifies the project name. For applications and OS Packages, it must be set to the name of the containing directory.

```
get_filename_component(SOS_NAME ${CMAKE_SOURCE_DIR} NAME)
```

For libraries, the name does not have to match the containing directory.

## SOS_SOURCELIST 

Applies to  applications, libraries, and OS packages.

This variables specifies a list of sources that are part of the project.

## SOS_OPTIMIZATION

Applies to applications, libraries, and OS packages.

This variable specifies the optimization level which is applied at compile time. The default value is `-Os`.


## SOS_DEFINITIONS

Applies to applications, libraries, and OS packages.

This variable contains a list of definitions that are passed to each source file at compile time.

```
set(SOS_DEFINITIONS -DMY_DEFINITION=1)
```

## SOS_INCLUDE_DIRECTORIES

Applies to applications, libraries, and OS packages.

This variable specifies a list of directories that are used with the `-I` flag at compile time.

```
set(SOS_INCLUDE_DIRECTORIES include)
```

## SOS_BUILD_FLAGS

Applies to applications, libraries, and OS packages.

This variable specifies build flags that are passed to the compiler.

```
set(SOS_BUILD_FLAGS -Werror)
```

## SOS_ARCH

Applies to applications, libraries, and OS packages.

Libraries and application are typically built for all supported architectures when using the `sos-app-std.cmake` and `sos-lib-std.cmake` files. For OS packages, the architecture must be set to match the chip for which the package is being built.

When building for `link` (native) architectures, this value is set to `link`. If the cmake build folder ends with `_link`, `SOS_BUILD_CONFIG` will be set to `link` and this value must be set to `link` as well.

When the cmake build folder ends with `_arm`, the following architectures can be used.

- `v7m`: ARM Cortex M3 (soft float)
- `v7em`: ARM Cortex M4 (soft float)
- `v7em_f4sh`: ARM Cortex M4F (single precision float, hard ABI)
- `v7em_f5sh`: ARM Cortex M47 (single precision float, hard ABI)
- `v7em_f5dh`: ARM Cortex M47 (double precision float, hard ABI)

## SOS_LIBRARIES

Applies to applications and OS packages.
 
This variable specifies a list of libraries to link. The `lib` part is dropped.

```
set(SOS_LIBRARIES jansson)
```

## SOS_CONFIG

Applies to applications, libraries, and OS packages.
 
This variable specifies the build configuration which is typically `debug` or `release` but must end with either `debug` or `release` (for example `uart_release` or `usb_release`).

## SOS_OPTION

Applies to L.
 
This variable specifies a library build option. This is an extension of `SOS_CONFIG` just for libraries. For example, some libraries can have different build options for either linking with the kernel or applications.

The resulting library is then called `${SOS_NAME}_${SOS_OPTION}_${SOS_CONFIG}`. If `SOS_OPTION` is empty (which is the default value), the library is just `${SOS_NAME}_${SOS_CONFIG}`.

The option is also passed as a define to each compilation unit preceeded by two underscores (`-D__${SOS_OPTION}`).

```
set(SOS_NAME test)
set(SOS_CONFIG release)
set(SOS_OPTION kernel)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)
set(SOS_CONFIG debug)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)
```

The above snippet will create library files with the format `test_kernel_release` and `test_kernel_debug`.


## SOS_DEVICE

Applies to OS packages.
 
This variable specifies the target device part number.

## SOS_LINKER_FLAGS

Applies to applications and OS packages.

This variable can be used to pass additional flags to the linker.
 
## SOS_RAM_SIZE

Applies to applications.
 
This variable sets the default RAM size. It must be a power of 2. This value serves as the default value for data as specified by the programmer. Whoever installs the program can override the default using `sl` or the `Sys::launch()` API.

## SOS_HARDWARD_ID

Applies to OS packages.
 
This variable specifies the board's hardware ID. This value must be provided by Stratify Labs, Inc.

## SOS_DEVICE_FAMILY

Applies to OS packages.
 
This variable specifies the device family.

## SOS_KERNEL_START_ADDRESS

Applies to OS packages.
 
This variable specifies the starting address of the kernel.

## SOS_BOOT_START_ADDRESS

Applies to OS packages.
 
This variable specifies the starting address of the kernel.

## SOS_BOOT_SOURCELIST

Applies to OS packages.
 
This variable specifies a list of sources for the bootloader. This is only used in conjunction with `sos-bsp-std.cmake`.

## SOS_KERNEL_SOURCELIST

Applies to OS packages.
 
This variable specifies a list of sources for the kernel. This is only used in conjunction with `sos-bsp-std.cmake`.

## SOS_ARCH_ARM_ALL

Applies to applications and libraries.

This variables tells `sos-app-std.cmake` or `sos-lib-std.cmake` to build all ARM architectures. The default value is `ON`.

## SOS_ARCH_ARM_V7M

Applies to applications and libraries.

This variables tells `sos-app-std.cmake` or `sos-lib-std.cmake` to build only the v7m architecture. The default value is `OFF`.  To build only this architecture, `SOS_ARCH_ARM_ALL` needs to be set to off and this variable set to `ON`.

## SOS_ARCH_ARM_V7EM

Applies to applications and libraries.

This variables tells `sos-app-std.cmake` or `sos-lib-std.cmake` to build only the v7em architecture. The default value is `OFF`.  To build only this architecture, `SOS_ARCH_ARM_ALL` needs to be set to off and this variable set to `ON`.

## SOS_ARCH_ARM_V7EM_F4SH

Applies to applications and libraries.

This variables tells `sos-app-std.cmake` or `sos-lib-std.cmake` to build only the v7em_f4sh architecture. The default value is `OFF`.  To build only this architecture, `SOS_ARCH_ARM_ALL` needs to be set to off and this variable set to `ON`.

## SOS_ARCH_ARM_V7EM_F5SH

Applies to applications and libraries.

This variables tells `sos-app-std.cmake` or `sos-lib-std.cmake` to build only the v7em_f5sh architecture. The default value is `OFF`.  To build only this architecture, `SOS_ARCH_ARM_ALL` needs to be set to off and this variable set to `ON`.

## SOS_ARCH_ARM_V7EM_F5DH

Applies to applications and libraries.

This variables tells `sos-app-std.cmake` or `sos-lib-std.cmake` to build only the v7em_f5dh architecture. The default value is `OFF`.  To build only this architecture, `SOS_ARCH_ARM_ALL` needs to be set to off and this variable set to `ON`.

## SOS_BUILD_C_FLAGS

Applies to applications, libraries, and OS packages.

This variables sets the default flags to the C compiler. You can append this variable but should only override it if you really know what you are doing.

```
# Appends Werror
set(SOS_BUILD_C_FLAGS ${SOS_BUILD_C_FLAGS} -Werror)
```

## SOS_BUILD_CXX_FLAGS

Applies to applications, libraries, and OS packages.

This variables sets the default flags to the C++ compiler. You can append this variable but should only override it if you really know what you are doing.

All `SOS_BUILD_C_FLAGS` are passed to the C++ compiler also.

```
# Appends Werror
set(SOS_BUILD_CXX_FLAGS ${SOS_BUILD_CXX_FLAGS} -Werror)
```

## SOS_BUILD_ASM_FLAGS

Applies to applications, libraries, and OS packages.

This variables sets the default flags to the C assembly. You can append this variable but should only override it if you really know what you are doing.



