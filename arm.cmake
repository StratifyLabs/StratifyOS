

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

sos_sdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_SOURCE_DIR}/include)

# Grab GLOB files - these don't affect the build so GLOB is OK
file(GLOB CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*)
file(GLOB LDSCRIPT_SOURCES ${CMAKE_SOURCE_DIR}/ldscript/*)

set(COMMON_SOURCES ${CMAKE_SOURCES} ${LDSCRIPT_SOURCES} ${SOS_INTERFACE_SOURCELIST})

#Add sys sources
sos_sdk_add_subdirectory(SYS_SOURCELIST src)

#Add sys sources
sos_sdk_add_subdirectory(CRT_SOURCELIST src/crt)

#Add bootloader sources
sos_sdk_add_subdirectory(BOOT_SOURCELIST src/boot)

set(SYS_INCLUDE_DIRECTORIES
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/sys/auth/tinycrypt/lib/include
	${CMAKE_SOURCE_DIR}/posix
	)

list(APPEND SYS_SOURCELIST ${COMMON_SOURCES})

if(BUILD_SYS OR BUILD_ALL)
	add_library(StratifyOS_sys_release_v7m STATIC)
	target_sources(StratifyOS_sys_release_v7m PRIVATE ${SYS_SOURCELIST})
	target_include_directories(StratifyOS_sys_release_v7m PRIVATE ${SYS_INCLUDE_DIRECTORIES})

	add_library(StratifyOS_sys_debug_v7m STATIC)
	sos_sdk_copy_target(StratifyOS_sys_release_v7m StratifyOS_sys_debug_v7m)

	sos_sdk_library_add_arm_targets(StratifyOS sys release)
	sos_sdk_library_add_arm_targets(StratifyOS sys debug)
endif()

if(BUILD_CRT OR BUILD_ALL)
	add_library(StratifyOS_crt_release_v7m STATIC)
	target_sources(StratifyOS_crt_release_v7m PRIVATE ${CRT_SOURCELIST})
	target_include_directories(StratifyOS_crt_release_v7m PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_options(StratifyOS_crt_release_v7m PUBLIC -mlong-calls)

	add_library(StratifyOS_crt_debug_v7m STATIC)
	sos_sdk_copy_target(StratifyOS_crt_release_v7m StratifyOS_crt_debug_v7m)

	# Create targets for all architectures using above settings
	sos_sdk_library_add_arm_targets(StratifyOS crt release)
	sos_sdk_library_add_arm_targets(StratifyOS crt debug)
endif()

#[[
if(BUILD_SYS OR BUILD_ALL)
	set(SOS_OPTION sys)
	set(SOS_CONFIG release)
	set(SOS_BUILD_FLAGS "")
	set(SOS_SOURCELIST ${SYS_SOURCELIST})
	include(sos-lib-std)

	set(SOS_OPTION sys)
	set(SOS_CONFIG debug)
	set(SOS_BUILD_FLAGS "")
	include(sos-lib-std)
endif()

if(BUILD_CRT OR BUILD_ALL)
	set(SOS_OPTION crt)
	set(SOS_CONFIG release)
	set(SOS_BUILD_FLAGS -mlong-calls)
	set(SOS_SOURCELIST ${CRT_SOURCELIST})
	include(sos-lib-std)
endif()

if(BUILD_BOOT OR BUILD_ALL)
	set(SOS_OPTION boot)
	set(SOS_CONFIG release)
	set(SOS_BUILD_FLAGS "")
	set(SOS_SOURCELIST ${BOOT_SOURCELIST} doxyfile)
	include(sos-lib-std)

	set(SOS_CONFIG debug)
	set(SOS_SOURCELIST ${BOOT_SOURCELIST})
	include(sos-lib-std)
endif()
#]]

install(DIRECTORY include/cortexm include/device include/mcu include/sos include/usbd DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY include/posix/ DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY ldscript/ DESTINATION lib/ldscripts PATTERN CMakelists.txt EXCLUDE)

option(SOS_SKIP_CMAKE "Dont install the cmake files" OFF)

if(NOT SOS_SKIP_CMAKE)
	install(DIRECTORY cmake/ DESTINATION ../cmake)
endif()

