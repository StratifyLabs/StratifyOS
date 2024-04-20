
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

cmsdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Grab GLOB files - these don't affect the build so GLOB is OK
file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*)
file(GLOB LDSCRIPT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ldscript/*)

set(COMMON_SOURCES
  ${CMAKE_SOURCES}
  ${LDSCRIPT_SOURCES}
  ${SOS_INTERFACE_SOURCELIST}
  ${SOS_CONFIG_PATH}/sos_config.h)

#Add sys sources
cmsdk_add_subdirectory(SYS_SOURCELIST src)
#Add sys sources
cmsdk_add_subdirectory(CRT_SOURCELIST src/crt)
#Add bootloader sources
cmsdk_add_subdirectory(BOOT_SOURCELIST src/boot)
#Add Cortex-m sources
cmsdk_add_subdirectory(CORTEXM_SOURCELIST src/cortexm)
#Add MCU sources
cmsdk_add_subdirectory(MCU_SOURCELIST src/mcu)
#Add Auth sources
cmsdk_add_subdirectory(AUTH_SOURCELIST src/auth)
#Add link transport
cmsdk_add_subdirectory(LINK_TRANSPORT_SOURCELIST src/link_transport)
#Add usbd
cmsdk_add_subdirectory(USBD_SOURCELIST src/usbd)
#add device library
cmsdk_add_subdirectory(DEVICE_SOURCELIST src/device)

set(SYS_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/posix>
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

set(COMPILE_DEFINITIONS_PRIVATE
  __PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  __PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  __PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  )

list(APPEND SYS_SOURCELIST ${COMMON_SOURCES})

cmsdk2_add_library(
  NAME StratifyOS
  OPTION interface
  CONFIG release
  TYPE INTERFACE
  ARCH ${CMSDK_ARCH}
  TARGET INTERFACE_RELEASE_TARGET)
cmsdk2_add_library(
  NAME StratifyOS
  OPTION interface
  TYPE INTERFACE
  CONFIG debug
  ARCH ${CMSDK_ARCH}
  TARGET INTERFACE_DEBUG_TARGET)
if(NOT DEFINED STRATIFYOS_LWIP_PATH)
  target_compile_definitions(${INTERFACE_RELEASE_TARGET} INTERFACE SOS_BOOTSTRAP_SOCKETS=${BOOTSTRAP_SOCKETS})
else()
  foreach(PATH ${STRATIFYOS_LWIP_PATH})
    target_include_directories(${INTERFACE_RELEASE_TARGET}
      INTERFACE
      $<BUILD_INTERFACE:${PATH}>
      )
  endforeach()
endif()
target_include_directories(${INTERFACE_RELEASE_TARGET}
  INTERFACE ${SYS_INCLUDE_INTERFACE_DIRECTORIES})
target_compile_definitions(${INTERFACE_RELEASE_TARGET}
  INTERFACE __StratifyOS__ CMSDK_BUILD_GIT_HASH=${CMSDK_GIT_HASH})
target_compile_options(${INTERFACE_RELEASE_TARGET}
  INTERFACE -Os
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-register -fno-exceptions -fno-threadsafe-statics>
  -mthumb -ffunction-sections -fdata-sections
  ${BUILD_FLOAT_OPTIONS})
cmsdk2_copy_target(
  SOURCE ${INTERFACE_RELEASE_TARGET}
  DESTINATION ${INTERFACE_DEBUG_TARGET})
cmsdk2_library_add_dependencies(
  TARGET ${INTERFACE_RELEASE_TARGET}
  TARGETS INTERFACE_RELEASE_TARGET_LIST)
cmsdk2_library_add_dependencies(
  TARGET ${INTERFACE_DEBUG_TARGET}
  TARGETS INTERFACE_DEBUG_TARGET_LIST)
foreach(RELEASE_TARGET ${INTERFACE_RELEASE_TARGET_LIST})
  get_target_property(ARCH ${RELEASE_TARGET} CMSDK_PROPERTY_ARCH)
  cmsdk2_get_arm_arch(
    ARCHITECTURE ${ARCH}
    FLOAT_OPTIONS BUILD_FLOAT_OPTIONS)
  target_compile_definitions(
    ${RELEASE_TARGET}
    INTERFACE ___release __${ARCH}=__${ARCH})
  target_compile_options(
    ${RELEASE_TARGET}
    INTERFACE ${BUILD_FLOAT_OPTIONS})
endforeach()
foreach(DEBUG_TARGET ${INTERFACE_DEBUG_TARGET_LIST})
  get_target_property(ARCH ${DEBUG_TARGET} CMSDK_PROPERTY_ARCH)
  cmsdk2_get_arm_arch(
    ARCHITECTURE ${ARCH}
    FLOAT_OPTIONS BUILD_FLOAT_OPTIONS)
  target_compile_definitions(
    ${DEBUG_TARGET}
    INTERFACE ___debug)
  target_compile_options(
    ${DEBUG_TARGET}
    INTERFACE ${BUILD_FLOAT_OPTIONS})
endforeach()

if(BUILD_MCU OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION mcu
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET MCU_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION mcu
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET MCU_DEBUG_TARGET)

  target_sources(${MCU_RELEASE_TARGET} PRIVATE ${MCU_SOURCELIST})
  target_include_directories(${MCU_RELEASE_TARGET}
    PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
    PRIVATE ${MCU_INCLUDE_DIRECTORIES})
  target_compile_definitions(${MCU_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  set_property(TARGET ${MCU_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
  cmsdk2_get_arm_arch(
    ARCHITECTURE ${CMSDK_ARCH}
    FLOAT_OPTIONS BUILD_FLOAT_OPTIONS
    FLOAT_DIRECTORY BUILD_FLOAT_DIRECTORY
    INSTALL_DIRECTORY BUILD_INSTALL_DIRECTORY)

  cmsdk2_copy_target(
    SOURCE ${MCU_RELEASE_TARGET}
    DESTINATION ${MCU_DEBUG_TARGET})
  cmsdk2_library_add_dependencies(
    TARGET ${MCU_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_interface)
  cmsdk2_library_add_dependencies(
    TARGET ${MCU_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_interface)

endif()

if(BUILD_CORTEXM OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION cortexm
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET CORTEXM_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION cortexm
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET CORTEXM_DEBUG_TARGET)

  target_sources(${CORTEXM_RELEASE_TARGET} PRIVATE ${CORTEXM_SOURCELIST})
  target_include_directories(${CORTEXM_RELEASE_TARGET} PRIVATE ${CORTEXM_INCLUDE_DIRECTORIES})
  target_compile_definitions(${CORTEXM_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  set_property(TARGET ${CORTEXM_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

  cmsdk2_copy_target(
    SOURCE ${CORTEXM_RELEASE_TARGET}
    DESTINATION ${CORTEXM_DEBUG_TARGET})
  cmsdk2_library_add_dependencies(
    TARGET ${CORTEXM_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_mcu)
  cmsdk2_library_add_dependencies(
    TARGET ${CORTEXM_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_mcu)
endif()

if(BUILD_LINK_TRANSPORT OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION linktransport
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET LINK_TRANSPORT_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION linktransport
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET LINK_TRANSPORT_DEBUG_TARGET)
  target_sources(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE ${LINK_TRANSPORT_SOURCELIST})
  target_include_directories(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE ${LINK_TRANSPORT_INCLUDE_DIRECTORIES})
  target_compile_definitions(${LINK_TRANSPORT_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  set_property(TARGET ${LINK_TRANSPORT_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

  cmsdk2_copy_target(
    SOURCE ${LINK_TRANSPORT_RELEASE_TARGET}
    DESTINATION ${LINK_TRANSPORT_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${LINK_TRANSPORT_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_interface)
  cmsdk2_library_add_dependencies(
    TARGET ${LINK_TRANSPORT_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_interface)
endif()

if(BUILD_DEVICE OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION device
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET DEVICE_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION device
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET DEVICE_DEBUG_TARGET)
  target_sources(${DEVICE_RELEASE_TARGET} PRIVATE ${DEVICE_SOURCELIST})
  target_include_directories(${DEVICE_RELEASE_TARGET} PRIVATE ${DEVICE_INCLUDE_DIRECTORIES})
  target_compile_definitions(${DEVICE_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  set_property(TARGET ${DEVICE_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

  cmsdk2_copy_target(
    SOURCE ${DEVICE_RELEASE_TARGET}
    DESTINATION ${DEVICE_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${DEVICE_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_interface)
  cmsdk2_library_add_dependencies(
    TARGET ${DEVICE_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_interface)
endif()

if(BUILD_USBD OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION usbd
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET USBD_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION usbd
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET USBD_DEBUG_TARGET)
  target_sources(${USBD_RELEASE_TARGET} PRIVATE ${USBD_SOURCELIST})
  target_include_directories(${USBD_RELEASE_TARGET} PRIVATE ${USBD_INCLUDE_DIRECTORIES})
  target_compile_definitions(${USBD_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  set_property(TARGET ${USBD_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

  cmsdk2_copy_target(
    SOURCE ${USBD_RELEASE_TARGET}
    DESTINATION ${USBD_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${USBD_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_interface)
  cmsdk2_library_add_dependencies(
    TARGET ${USBD_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_interface)
endif()

if(BUILD_MCU OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION auth
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET AUTH_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION auth
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET AUTH_DEBUG_TARGET)

  target_sources(${AUTH_RELEASE_TARGET} PRIVATE ${AUTH_SOURCELIST})
  target_include_directories(${AUTH_RELEASE_TARGET}
    PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
    PRIVATE ${AUTH_INCLUDE_DIRECTORIES})
  target_compile_definitions(${AUTH_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  target_compile_options(${AUTH_RELEASE_TARGET} PUBLIC PRIVATE -mpure-code)
  set_property(TARGET ${AUTH_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

  cmsdk2_copy_target(
    SOURCE ${AUTH_RELEASE_TARGET}
    DESTINATION ${AUTH_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${AUTH_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_interface)
  cmsdk2_library_add_dependencies(
    TARGET ${AUTH_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_interface)
endif()

if(BUILD_SYS OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION sys
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET SYS_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION sys
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET SYS_DEBUG_TARGET)

  target_sources(${SYS_RELEASE_TARGET} PRIVATE ${SYS_SOURCELIST})
  target_include_directories(${SYS_RELEASE_TARGET}
    PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
    PRIVATE ${SYS_INCLUDE_DIRECTORIES})
  target_compile_definitions(${SYS_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  set_property(TARGET ${SYS_RELEASE_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)

  cmsdk2_copy_target(
    SOURCE ${SYS_RELEASE_TARGET}
    DESTINATION ${SYS_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${SYS_RELEASE_TARGET}
    DEPENDENCIES ${SYS_DEPENDENCIES})
  cmsdk2_library_add_dependencies(
    TARGET ${SYS_DEBUG_TARGET}
    DEPENDENCIES ${SYS_DEPENDENCIES})
endif()

if(BUILD_CRT OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION crt
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET CRT_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION crt
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET CRT_DEBUG_TARGET)
  target_sources(${CRT_RELEASE_TARGET} PRIVATE ${CRT_SOURCELIST})
  target_include_directories(${CRT_RELEASE_TARGET}
    PUBLIC ${SYS_INCLUDE_INTERFACE_DIRECTORIES}
    PRIVATE ${SYS_INCLUDE_DIRECTORIES})
  target_compile_definitions(
    ${CRT_RELEASE_TARGET}
    PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})

  target_compile_options(
    ${CRT_RELEASE_TARGET}
    PUBLIC -mlong-calls)

  cmsdk2_copy_target(
    SOURCE ${CRT_RELEASE_TARGET}
    DESTINATION ${CRT_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${CRT_RELEASE_TARGET}
    DEPENDENCIES StratifyOS_interface)
  cmsdk2_library_add_dependencies(
    TARGET ${CRT_DEBUG_TARGET}
    DEPENDENCIES StratifyOS_interface)

endif()

if(BUILD_BOOT OR BUILD_ALL)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION boot
    CONFIG release
    ARCH ${CMSDK_ARCH}
    TARGET BOOT_RELEASE_TARGET)
  cmsdk2_add_library(
    NAME StratifyOS
    OPTION boot
    CONFIG debug
    ARCH ${CMSDK_ARCH}
    TARGET BOOT_DEBUG_TARGET)
  target_sources(${BOOT_RELEASE_TARGET} PRIVATE ${BOOT_SOURCELIST})
  target_include_directories(${BOOT_RELEASE_TARGET} PRIVATE ${SYS_INCLUDE_DIRECTORIES})
  target_compile_definitions(${BOOT_RELEASE_TARGET} PRIVATE ${COMPILE_DEFINITIONS_PRIVATE})
  target_compile_options(${BOOT_RELEASE_TARGET} PUBLIC -Os)

  cmsdk2_copy_target(
    SOURCE ${BOOT_RELEASE_TARGET}
    DESTINATION ${BOOT_DEBUG_TARGET})

  cmsdk2_library_add_dependencies(
    TARGET ${BOOT_RELEASE_TARGET}
    DEPENDENCIES ${BOOT_DEPENDENCIES})
  cmsdk2_library_add_dependencies(
    TARGET ${BOOT_DEBUG_TARGET}
    DEPENDENCIES ${BOOT_DEPENDENCIES})
endif()

install(DIRECTORY include/cortexm include/device include/mcu include/sos include/usbd DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY include/posix/ DESTINATION include PATTERN CMakelists.txt EXCLUDE)
install(DIRECTORY ldscript/ DESTINATION lib/ldscripts PATTERN CMakelists.txt EXCLUDE)

#/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/local/bin/arm-none-eabi-gcc -DCMSDK_BUILD_GIT_HASH=225c186 -D__PROJECT_VERSION_MAJOR=4 -D__PROJECT_VERSION_MINOR=2 -D__PROJECT_VERSION_PATCH=0 -D___debug -D__v7em_f4sh -I/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/dependencies/StratifyOS/src -I/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/dependencies/StratifyOS/include -I/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/dependencies/StratifyOS/include/posix -I/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/config -I/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/dependencies/InetAPI/lwip/include -I/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/dependencies/InetAPI/lwip/lwip-2.1.2/src/include -Os -mthumb -ffunction-sections -fdata-sections -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_PRESENT=1 -DARM_MATH_CM4=1 -MD -MT SDK/StratifyOS/CMakeFiles/StratifyOS_cortexm_debug_v7em_f4sh.dir/src/cortexm/devfs.c.obj -MF SDK/StratifyOS/CMakeFiles/StratifyOS_cortexm_debug_v7em_f4sh.dir/src/cortexm/devfs.c.obj.d -o SDK/StratifyOS/CMakeFiles/StratifyOS_cortexm_debug_v7em_f4sh.dir/src/cortexm/devfs.c.obj -c /Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/dependencies/StratifyOS/src/cortexm/devfs.c