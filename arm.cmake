
option(BUILD_ALL "Build All configurations" ON)
option(BUILD_SYS "Build Stratify OS System library" OFF)
option(BUILD_CRT "Build C Runtime library" OFF)
option(BUILD_BOOT "Build Bootloader library" OFF)

#check for LWIP
include(CheckIncludeFiles)

if(NOT STRATIFYOS_LWIP_PATH)
	message(STATUS "LWIP path not provided -- check system for LWIP")
	check_include_files("lwip/sockets.h" HAVE_LWIP_SOCKETS_H)
	check_include_files("mbedtls/net_sockets.h" HAVE_MBEDTLS_NETSOCKETS_H)
	if(NOT HAVE_LWIP_SOCKETS_H)
		message(STATUS "Bootstrapping sockets")
		set(BOOTSTRAP_SOCKETS 1)
	else()
		set(BOOTSTRAP_SOCKETS 0)
	endif()
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
	${CMAKE_CURRENT_SOURCE_DIR}/include/posix
	${CMAKE_CURRENT_SOURCE_DIR}/include
	)

list(APPEND SYS_SOURCELIST ${COMMON_SOURCES})

macro(add_lwip_path TARGET DOMAIN)
	if(NOT STRATIFYOS_LWIP_PATH)
		target_compile_definitions(${TARGET} ${DOMAIN} SOS_BOOTSTRAP_SOCKETS=${BOOTSTRAP_SOCKETS})
	else()
		target_include_directories(${TARGET}
			${DOMAIN}
			$<BUILD_INTERFACE:${STRATIFYOS_LWIP_PATH}>
			)
	endif()
endmacro()

macro(create_iface_library CONFIG CONFIG_LOWER ARCH)
	sos_sdk_library_target(IFACE_${CONFIG} StratifyOS iface ${CONFIG_LOWER} ${ARCH})
	add_library(${IFACE_${CONFIG}_TARGET} INTERFACE)
	target_include_directories(${IFACE_${CONFIG}_TARGET}
		INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/posix>
		)

	add_lwip_path(${IFACE_${CONFIG}_TARGET} INTERFACE)

	install(
		TARGETS ${IFACE_${CONFIG}_TARGET}
		EXPORT ${IFACE_${CONFIG}_TARGET}
		DESTINATION lib
		OPTIONAL)
	install(
		EXPORT ${IFACE_${CONFIG}_TARGET}
		DESTINATION cmake/targets)
endmacro()

create_iface_library(RELEASE release ${SOS_ARCH})
create_iface_library(DEBUG debug ${SOS_ARCH})
foreach(ARCH ${SOS_ARCH_LIST})
	create_iface_library(RELEASE release ${ARCH})
	create_iface_library(DEBUG debug ${ARCH})
endforeach()


if(BUILD_SYS OR BUILD_ALL)
	sos_sdk_library_target(SYS_RELEASE StratifyOS sys release ${SOS_ARCH})
	sos_sdk_library_target(SYS_DEBUG StratifyOS sys debug ${SOS_ARCH})

	add_library(${SYS_RELEASE_TARGET} STATIC)
	target_sources(${SYS_RELEASE_TARGET} PRIVATE ${SYS_SOURCELIST})
	target_include_directories(${SYS_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_options(${SYS_RELEASE_TARGET} PUBLIC -Os)
	add_lwip_path(${SYS_RELEASE_TARGET} PUBLIC)
	set_property(TARGET ${SYS_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${SYS_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${SYS_RELEASE_TARGET} ${SYS_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${SYS_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${SYS_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_CRT OR BUILD_ALL)
	sos_sdk_library_target(CRT_RELEASE StratifyOS crt release ${SOS_ARCH})
	sos_sdk_library_target(CRT_DEBUG StratifyOS crt debug ${SOS_ARCH})

	add_library(${CRT_RELEASE_TARGET} STATIC)
	target_sources(${CRT_RELEASE_TARGET} PRIVATE ${CRT_SOURCELIST})
	target_include_directories(${CRT_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_definitions(${CRT_RELEASE_TARGET} PUBLIC __StratifyOS__)
	add_lwip_path(${CRT_RELEASE_TARGET} PUBLIC)
	target_compile_options(${CRT_RELEASE_TARGET} PUBLIC -mlong-calls PRIVATE -Os)

	add_library(${CRT_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${CRT_RELEASE_TARGET} ${CRT_DEBUG_TARGET})

	# Create targets for all architectures using above settings
	sos_sdk_library_add_arch_targets("${CRT_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${CRT_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_BOOT OR BUILD_ALL)
	sos_sdk_library_target(BOOT_RELEASE StratifyOS boot release ${SOS_ARCH})
	sos_sdk_library_target(BOOT_DEBUG StratifyOS boot debug ${SOS_ARCH})

	add_library(${BOOT_RELEASE_TARGET} STATIC)
	target_sources(${BOOT_RELEASE_TARGET} PRIVATE ${BOOT_SOURCELIST})
	target_include_directories(${BOOT_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_options(${BOOT_RELEASE_TARGET} PUBLIC -Os)
	add_lwip_path(${BOOT_RELEASE_TARGET} PUBLIC)

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

install(FILES StratifyOS.cmake
	DESTINATION cmake/targets)

if(SOS_CREATE_GCC_HARD)
	install(CODE "include(../create-gcc-hard.cmake)")
endif()

