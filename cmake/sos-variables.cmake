
option(SOS_VERBOSE "Create Verbose Makefiles | Application, OS Package, Library" OFF)

set(SOS_NAME "" CACHE STRING "Specify the project name | Application, OS Package, Library")
set(SOS_SOURCELIST "" CACHE STRING "Specify the list of sources to build (required) Application, OS Package, Library")
set(SOS_OPTIMIZATION -Os CACHE STRING "Compile optimization level | Application, OS Package, Library")
set(SOS_DEFINITIONS "" CACHE STRING "Specify definitions values to pass while building | Application, OS Package, Library")
set(SOS_INCLUDE_DIRECTORIES "" CACHE STRING "Specify additional include directories for use while building | Application, OS Package, Library")
set(SOS_BUILD_FLAGS "" CACHE STRING "Specify additional build flags | Application, OS Package, Library")
set(SOS_ARCH "" CACHE STRING "Specify target ARM architecture | Application, OS Package, Library")
set(SOS_LIBRARIES "" CACHE STRING "Specify additional libraries to link | Application, OS Package")
set(SOS_CONFIG "release" CACHE STRING "Specify a library configuration (debug or release) | Application, OS Package, Library")
set(SOS_OPTION "" CACHE STRING "Specify a library option for building which supplements the SOS_CONFIG value to further differentiate a build | Library")
set(SOS_DEVICE "" CACHE STRING "Specify the device | OS Package")
set(SOS_LINKER_FLAGS "" CACHE STRING "Specify additional linker flags | Application, OS Package")
set(SOS_RAM_SIZE "" CACHE STRING "Specify the default data size for the program | Application")
set(SOS_HARDWARD_ID "" CACHE STRING "Specify the hardware ID for the package | OS Package")
set(SOS_DEVICE_FAMILY "" CACHE STRING "Specify the device family | OS Package")
set(SOS_KERNEL_START_ADDRESS "" CACHE STRING "Specify the start address for the kernel | OS Package")
set(SOS_BOOT_START_ADDRESS "" CACHE STRING "Specify the start address for the bootloader | OS Package")
set(SOS_BOOT_SOURCELIST "" CACHE STRING "Specify the list of sources to build the kernel (use with sos-bsp-std.cmake)| OS Package")
set(SOS_KERNEL_SOURCELIST "" CACHE STRING "Specify the list of sources to build the bootloader (use with sos-bsp-std.cmake)| OS Package")

# Used with sos-app-std, sos-lib-std
option(SOS_ARCH_ARM_ALL "Build All configurations | use with sos-app-std.cmake or sos-lib-std.cmake to auto set SOS_ARCH" ON)
option(SOS_ARCH_ARM_V7M "Build v7m configuration  | use with sos-app-std.cmake or sos-lib-std.cmake to auto set SOS_ARCH" OFF)
option(SOS_ARCH_ARM_V7EM "Build v7em configuration  | use with sos-app-std.cmake or sos-lib-std.cmake to auto set SOS_ARCH" OFF)
option(SOS_ARCH_ARM_V7EM_F4SH "Build v7em_f4sh configuration | use with sos-app-std.cmake or sos-lib-std.cmake to auto set SOS_ARCH" OFF)
option(SOS_ARCH_ARM_V7EM_F5SH "Build v7em_f5sh configuration | use with sos-app-std.cmake or sos-lib-std.cmake to auto set SOS_ARCH" OFF)
option(SOS_ARCH_ARM_V7EM_F5DH "Build v7em_f5dh configuration | use with sos-app-std.cmake or sos-lib-std.cmake to auto set SOS_ARCH" OFF)


set(SOS_BUILD_C_FLAGS "-Wall -mthumb -D__StratifyOS__ -ffunction-sections -fdata-sections -fomit-frame-pointer" CACHE STRING "C Flags for building | Application, OS Package, Library")
set(SOS_BUILD_CXX_FLAGS "-fno-exceptions -fno-unwind-tables" CACHE STRING "CXX Flags for building | Application, OS Package, Library")
set(SOS_BUILD_ASM_FLAGS "-mthumb" CACHE STRING "ASM build flags | Application, OS Package, Library")

#Resolve the build configuration based on the folder name suffix _arm or _link
string(LENGTH ${CMAKE_BINARY_DIR} STR_LENGTH)
string(FIND ${CMAKE_BINARY_DIR} "_link" LINK_POS REVERSE)
string(FIND ${CMAKE_BINARY_DIR} "_arm" ARM_POS)

math(EXPR IS_LINK "${STR_LENGTH} - ${LINK_POS}")
math(EXPR IS_ARM "${STR_LENGTH} - ${ARM_POS}")

if( ${IS_LINK} STREQUAL 5 )
	set(SOS_BUILD_CONFIG link CACHE INTERNAL "sos build config is link")
	message( STATUS "Set build config to link" )
elseif( ${IS_ARM} STREQUAL 4 )
	set(SOS_BUILD_CONFIG arm CACHE INTERNAL "sos build config is link")
else()
	message( FATAL_ERROR "No Configuration available build in *_link or *_arm directory or say -DSOS_BUILD_CONFIG:STRING=<arm|link>")
endif()
