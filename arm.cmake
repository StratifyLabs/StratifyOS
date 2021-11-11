
option(BUILD_ALL "Build All configurations" ON)
option(BUILD_MCU "Build Stratify OS MCU Helper library" OFF)
option(BUILD_AUTH "Build Stratify OS Authentication library" OFF)
option(BUILD_CORTEXM "Build Stratify OS Cortex M library" OFF)
option(BUILD_SYS "Build Stratify OS System library" OFF)
option(BUILD_CRT "Build C Runtime library" OFF)
option(BUILD_BOOT "Build Bootloader library" OFF)
option(BUILD_DEVICE "Build Device library" OFF)
option(BUILD_USBD "Build USB Device library" OFF)
option(BUILD_LINK_TRANSPORT "Build Link transport library" OFF)

#check for LWIP
if(NOT DEFINED STRATIFYOS_LWIP_PATH)
	message(STATUS "No LWIP, Bootstrapping Sockets")
	set(BOOTSTRAP_SOCKETS 1)
else()
	message(STATUS "LWIP is installed")
	set(BOOTSTRAP_SOCKETS 0)
endif()


if(NOT SOS_CONFIG_PATH)
	message(FATAL_ERROR "SOS_CONFIG_PATH must provide directory to `sos_config.h`")
endif()

sos_sdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Grab GLOB files - these don't affect the build so GLOB is OK
file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*)
file(GLOB LDSCRIPT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ldscript/*)

set(COMMON_SOURCES
	${CMAKE_SOURCES}
	${LDSCRIPT_SOURCES}
	${SOS_INTERFACE_SOURCELIST}
	${SOS_CONFIG_PATH}/sos_config.h)

#Add sys sources
sos_sdk_add_subdirectory(SYS_SOURCELIST src)

#Add sys sources
sos_sdk_add_subdirectory(CRT_SOURCELIST src/crt)

#Add bootloader sources
sos_sdk_add_subdirectory(BOOT_SOURCELIST src/boot)

#Add Cortex-m sources
sos_sdk_add_subdirectory(CORTEXM_SOURCELIST src/cortexm)

#Add MCU sources
sos_sdk_add_subdirectory(MCU_SOURCELIST src/mcu)


#Add Auth sources
sos_sdk_add_subdirectory(AUTH_SOURCELIST src/auth)

#Add link transport
sos_sdk_add_subdirectory(LINK_TRANSPORT_SOURCELIST src/link_transport)

#Add usbd
sos_sdk_add_subdirectory(USBD_SOURCELIST src/usbd)

#add device library
sos_sdk_add_subdirectory(DEVICE_SOURCELIST src/device)


set(SYS_INCLUDE_DIRECTORIES
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/include
	$<BUILD_INTERFACE:${SOS_CONFIG_PATH}>
	)

set(SYS_INCLUDE_INTERFACE_DIRECTORIES
	$<BUILD_INTERFACE:${SOS_CONFIG_PATH}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/posix>
	)

set(LINK_TRANSPORT_INCLUDE_DIRECTORIES
	${SYS_INCLUDE_DIRECTORIES}
	)

set(CORTEXM_INCLUDE_DIRECTORIES
	${SYS_INCLUDE_DIRECTORIES}
	)

set(DEVICE_INCLUDE_DIRECTORIES
	${SYS_INCLUDE_DIRECTORIES}
	)

set(USBD_INCLUDE_DIRECTORIES
	${SYS_INCLUDE_DIRECTORIES}
	)

set(MCU_INCLUDE_DIRECTORIES
	${SYS_INCLUDE_DIRECTORIES}
	)

set(SYS_DEPENDENCIES
	StratifyOS_linktransport
	StratifyOS_device
	StratifyOS_cortexm
	StratifyOS_auth
	compiler_rt)

set(BOOT_DEPENDENCIES
	StratifyOS_linktransport
	StratifyOS_device
	StratifyOS_cortexm
	StratifyOS_mcu
	StratifyOS_auth
	newlib_libc
	newlib_libm
	compiler_rt)

set(COMPILE_OPTIONS	$<$<COMPILE_LANGUAGE:CXX>:-Wno-register>)
set(COMPILE_DEFINITIONS_PRIVATE
	__PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
	__PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
	__PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
	)

list(APPEND SYS_SOURCELIST ${COMMON_SOURCES})

macro(add_lwip_path TARGET DOMAIN)
	if(NOT DEFINED STRATIFYOS_LWIP_PATH)
		target_compile_definitions(${TARGET} ${DOMAIN} SOS_BOOTSTRAP_SOCKETS=${BOOTSTRAP_SOCKETS})
	else()
		string(COMPARE EQUAL ${DOMAIN} PRIVATE IS_PRIVATE)
		if(IS_PRIVATE)
			target_include_directories(${TARGET}
				PRIVATE
				${STRATIFYOS_LWIP_PATH}
				)
		else()
			foreach(PATH ${STRATIFYOS_LWIP_PATH})
				target_include_directories(${TARGET}
					PUBLIC
					$<BUILD_INTERFACE:${PATH}>
					)
			endforeach()
		endif()

	endif()
endmacro()

if(BUILD_CORTEXM OR BUILD_ALL)
	sos_sdk_library_target(CORTEXM_RELEASE StratifyOS cortexm release ${SOS_ARCH})
	sos_sdk_library_target(CORTEXM_DEBUG StratifyOS cortexm debug ${SOS_ARCH})

	add_library(${CORTEXM_RELEASE_TARGET} STATIC)
	target_sources(${CORTEXM_RELEASE_TARGET} PRIVATE ${CORTEXM_SOURCELIST})
	target_include_directories(${CORTEXM_RELEASE_TARGET} PRIVATE ${CORTEXM_INCLUDE_DIRECTORIES})
	target_compile_definitions(${CORTEXM_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${CORTEXM_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS})
	add_lwip_path(${CORTEXM_RELEASE_TARGET} PRIVATE)
	set_property(TARGET ${CORTEXM_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${CORTEXM_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${CORTEXM_RELEASE_TARGET} ${CORTEXM_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${CORTEXM_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${CORTEXM_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_LINK_TRANSPORT OR BUILD_ALL)
	sos_sdk_library_target(LINK_TRANSPORT_RELEASE StratifyOS linktransport release ${SOS_ARCH})
	sos_sdk_library_target(LINK_TRANSPORT_DEBUG StratifyOS linktransport debug ${SOS_ARCH})

	add_library(${LINK_TRANSPORT_RELEASE_TARGET} STATIC)
	target_sources(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE ${LINK_TRANSPORT_SOURCELIST})
	target_include_directories(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE ${LINK_TRANSPORT_INCLUDE_DIRECTORIES})
	target_compile_definitions(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${LINK_TRANSPORT_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS})
	add_lwip_path(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE)
	set_property(TARGET ${LINK_TRANSPORT_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${LINK_TRANSPORT_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${LINK_TRANSPORT_RELEASE_TARGET} ${LINK_TRANSPORT_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${LINK_TRANSPORT_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${LINK_TRANSPORT_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_DEVICE OR BUILD_ALL)
	sos_sdk_library_target(DEVICE_RELEASE StratifyOS device release ${SOS_ARCH})
	sos_sdk_library_target(DEVICE_DEBUG StratifyOS device debug ${SOS_ARCH})

	add_library(${DEVICE_RELEASE_TARGET} STATIC)
	target_sources(${DEVICE_RELEASE_TARGET} PRIVATE ${DEVICE_SOURCELIST})
	target_include_directories(${DEVICE_RELEASE_TARGET} PRIVATE ${DEVICE_INCLUDE_DIRECTORIES})
	target_compile_definitions(${DEVICE_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${DEVICE_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS})
	add_lwip_path(${DEVICE_RELEASE_TARGET} PRIVATE)
	set_property(TARGET ${DEVICE_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${DEVICE_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${DEVICE_RELEASE_TARGET} ${DEVICE_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${DEVICE_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${DEVICE_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_USBD OR BUILD_ALL)
	sos_sdk_library_target(USBD_RELEASE StratifyOS usbd release ${SOS_ARCH})
	sos_sdk_library_target(USBD_DEBUG StratifyOS usbd debug ${SOS_ARCH})

	add_library(${USBD_RELEASE_TARGET} STATIC)
	target_sources(${USBD_RELEASE_TARGET} PRIVATE ${USBD_SOURCELIST})
	target_include_directories(${USBD_RELEASE_TARGET} PRIVATE ${USBD_INCLUDE_DIRECTORIES})
	target_compile_definitions(${USBD_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${USBD_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS})
	add_lwip_path(${USBD_RELEASE_TARGET} PRIVATE)
	set_property(TARGET ${USBD_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${USBD_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${USBD_RELEASE_TARGET} ${USBD_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${USBD_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${USBD_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_MCU OR BUILD_ALL)
	sos_sdk_library_target(AUTH_RELEASE StratifyOS auth release ${SOS_ARCH})
	sos_sdk_library_target(AUTH_DEBUG StratifyOS auth debug ${SOS_ARCH})

	add_library(${AUTH_RELEASE_TARGET} STATIC)
	target_sources(${AUTH_RELEASE_TARGET} PRIVATE ${AUTH_SOURCELIST})
	target_include_directories(${AUTH_RELEASE_TARGET}
		PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
		PRIVATE ${AUTH_INCLUDE_DIRECTORIES})
	target_compile_definitions(${AUTH_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${AUTH_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS} PRIVATE -mpure-code)
	add_lwip_path(${AUTH_RELEASE_TARGET} PUBLIC)
	set_property(TARGET ${AUTH_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${AUTH_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${AUTH_RELEASE_TARGET} ${AUTH_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${AUTH_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${AUTH_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()


if(BUILD_MCU OR BUILD_ALL)
	sos_sdk_library_target(MCU_RELEASE StratifyOS mcu release ${SOS_ARCH})
	sos_sdk_library_target(MCU_DEBUG StratifyOS mcu debug ${SOS_ARCH})

	add_library(${MCU_RELEASE_TARGET} STATIC)
	target_sources(${MCU_RELEASE_TARGET} PRIVATE ${MCU_SOURCELIST})
	target_include_directories(${MCU_RELEASE_TARGET}
		PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
		PRIVATE ${MCU_INCLUDE_DIRECTORIES})
	target_compile_definitions(${MCU_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${MCU_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS})
	add_lwip_path(${MCU_RELEASE_TARGET} PUBLIC)
	set_property(TARGET ${MCU_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${MCU_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${MCU_RELEASE_TARGET} ${MCU_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${MCU_RELEASE_OPTIONS}" ${SOS_ARCH} "")
	sos_sdk_library_add_arch_targets("${MCU_DEBUG_OPTIONS}" ${SOS_ARCH} "")
endif()

if(BUILD_SYS OR BUILD_ALL)
	sos_sdk_library_target(SYS_RELEASE StratifyOS sys release ${SOS_ARCH})
	sos_sdk_library_target(SYS_DEBUG StratifyOS sys debug ${SOS_ARCH})

	add_library(${SYS_RELEASE_TARGET} STATIC)
	target_sources(${SYS_RELEASE_TARGET} PRIVATE ${SYS_SOURCELIST})
	target_include_directories(${SYS_RELEASE_TARGET}
		PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
		PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_definitions(${SYS_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${SYS_RELEASE_TARGET} PUBLIC -Os ${COMPILE_OPTIONS})
	add_lwip_path(${SYS_RELEASE_TARGET} PUBLIC)
	set_property(TARGET ${SYS_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

	add_library(${SYS_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${SYS_RELEASE_TARGET} ${SYS_DEBUG_TARGET})

	sos_sdk_library_add_arch_targets("${SYS_RELEASE_OPTIONS}" ${SOS_ARCH} "${SYS_DEPENDENCIES}")
	sos_sdk_library_add_arch_targets("${SYS_DEBUG_OPTIONS}" ${SOS_ARCH} "${SYS_DEPENDENCIES}")
endif()


if(BUILD_CRT OR BUILD_ALL)
	sos_sdk_library_target(CRT_RELEASE StratifyOS crt release ${SOS_ARCH})
	sos_sdk_library_target(CRT_DEBUG StratifyOS crt debug ${SOS_ARCH})

	add_library(${CRT_RELEASE_TARGET} STATIC)
	target_sources(${CRT_RELEASE_TARGET} PRIVATE ${CRT_SOURCELIST})
	target_include_directories(${CRT_RELEASE_TARGET}
		PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
		PRIVATE ${SYS_INCLUDE_DIRECTORIES})
	target_compile_definitions(${CRT_RELEASE_TARGET} PUBLIC __StratifyOS__ PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	add_lwip_path(${CRT_RELEASE_TARGET} PUBLIC)
	target_compile_options(${CRT_RELEASE_TARGET} PUBLIC -mlong-calls ${COMPILE_OPTIONS} PRIVATE -Os)

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
	target_compile_definitions(${BOOT_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
	target_compile_options(${BOOT_RELEASE_TARGET} PUBLIC -Os)
	add_lwip_path(${BOOT_RELEASE_TARGET} PUBLIC)

	add_library(${BOOT_DEBUG_TARGET} STATIC)
	sos_sdk_copy_target(${BOOT_RELEASE_TARGET} ${BOOT_DEBUG_TARGET})

	# Create targets for all architectures using above settings
	sos_sdk_library_add_arch_targets("${BOOT_RELEASE_OPTIONS}" ${SOS_ARCH} "${BOOT_DEPENDENCIES}")
	sos_sdk_library_add_arch_targets("${BOOT_DEBUG_OPTIONS}" ${SOS_ARCH} "${BOOT_DEPENDENCIES}")
endif()

install(DIRECTORY include/cortexm include/device include/mcu include/sos include/usbd DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY include/posix/ DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY ldscript/ DESTINATION lib/ldscripts PATTERN CMakelists.txt EXCLUDE)


