
option(BUILD_ALL "Build All configurations" ON)
option(BUILD_SYS "Build Stratify OS System library" OFF)
option(BUILD_CRT "Build C Runtime library" OFF)
option(BUILD_BOOT "Build Bootloader library" OFF)

#check for LWIP
include(CheckIncludeFiles)
check_include_files("lwip/sockets.h" HAVE_LWIP_SOCKETS_H)
check_include_files("mbedtls/net_sockets.h" HAVE_MBEDTLS_NETSOCKETS_H)
if(NOT HAVE_LWIP_SOCKETS_H)
	set(SOS_DEFINITIONS SOS_BOOTSTRAP_SOCKETS=1)
endif()

sos_sdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Grab GLOB files - these don't affect the build so GLOB is OK
file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*)
file(GLOB LDSCRIPT_SOURCES ${CMAKE_SOURCE_DIR}/ldscript/*)

set(COMMON_SOURCES ${CMAKE_SOURCES} ${LDSCRIPT_SOURCES} ${SOS_INTERFACE_SOURCELIST})

#Add sys sources
sos_sdk_add_subdirectory(SYS_SOURCELIST src)

#Add sys sources
sos_sdk_add_subdirectory(CRT_SOURCELIST src/crt)

#Add bootloader sources
sos_sdk_add_subdirectory(BOOT_SOURCELIST src/boot)

set(SYS_INCLUDE_DIRECTORIES
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/sys/auth/tinycrypt/lib/include
	${CMAKE_CURRENT_SOURCE_DIR}/posix
	)

list(APPEND SYS_SOURCELIST ${COMMON_SOURCES})

if(BUILD_SYS OR BUILD_ALL)
	sos_sdk_library_target(SYS_RELEASE StratifyOS sys release v7m)
	sos_sdk_library_target(SYS_DEBUG StratifyOS sys debug v7m)

	add_library(${SYS_RELEASE_TARGET} STATIC)
	target_sources(${SYS_RELEASE_TARGET} PRIVATE ${SYS_SOURCELIST})
	target_include_directories(${SYS_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_options(${SYS_RELEASE_TARGET} PUBLIC -Os)
	set_property(TARGET ${SYS_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${SYS_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${SYS_RELEASE_TARGET} ${SYS_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${SYS_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${SYS_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_CRT OR BUILD_ALL)
	sos_sdk_library_target(CRT_RELEASE StratifyOS crt release v7m)
	sos_sdk_library_target(CRT_DEBUG StratifyOS crt debug v7m)

	add_library(${CRT_RELEASE_TARGET} STATIC)
	target_sources(${CRT_RELEASE_TARGET} PRIVATE ${CRT_SOURCELIST})
	target_include_directories(${CRT_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_definitions(${CRT_RELEASE_TARGET} PUBLIC __StratifyOS__)
	target_compile_options(${CRT_RELEASE_TARGET} PUBLIC -mlong-calls PRIVATE -Os)

	add_library(${CRT_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${CRT_RELEASE_TARGET} ${CRT_DEBUG_TARGET})

	# Create targets for all architectures using above settings
	sos_sdk_library_add_arch_targets("${CRT_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${CRT_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_BOOT OR BUILD_ALL)
	sos_sdk_library_target(BOOT_RELEASE StratifyOS boot release v7m)
	sos_sdk_library_target(BOOT_DEBUG StratifyOS boot debug v7m)

	add_library(${BOOT_RELEASE_TARGET} STATIC)
	target_sources(${BOOT_RELEASE_TARGET} PRIVATE ${BOOT_SOURCELIST})
	target_include_directories(${BOOT_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_options(${BOOT_RELEASE_TARGET} PUBLIC -Os)

	add_library(${BOOT_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${BOOT_RELEASE_TARGET} ${BOOT_DEBUG_TARGET})

	# Create targets for all architectures using above settings
	sos_sdk_library_add_arch_targets("${BOOT_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${BOOT_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

install(DIRECTORY include/cortexm include/device include/mcu include/sos include/usbd DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY include/posix/ DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY ldscript/ DESTINATION lib/ldscripts PATTERN CMakelists.txt EXCLUDE)

option(SOS_SKIP_CMAKE "Dont install the cmake files" OFF)
option(SOS_CREATE_GCC_HARD "Dont install the cmake files" OFF)

if(NOT SOS_SKIP_CMAKE)
	install(DIRECTORY cmake/ DESTINATION ../cmake)
endif()

if(SOS_CREATE_GCC_HARD)
	install(CODE "include(../create-gcc-hard.cmake)")
endif()

