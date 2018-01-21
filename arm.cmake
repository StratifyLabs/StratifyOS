

#check for LWIP
include(CheckIncludeFiles)
check_include_files("lwip/sockets.h" HAVE_LWIP_SOCKETS_H)
if(HAVE_LWIP_SOCKETS_H)
  set(SOS_LIB_BUILD_FLAGS -DHAVE_LWIP_SOCKETS_H=1)
endif()

#Add sys sources
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src CACHE INTERNAL "src prefix")
add_subdirectory(src)
set(SYS_SOURCELIST ${SOURCES})

#Add sys sources
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src/crt CACHE INTERNAL "src crt prefix")
add_subdirectory(src/crt)
set(CRT_SOURCELIST ${SOURCES})

set(SOS_LIB_INCLUDE_DIRECTORIES src include include/posix CACHE INTERNAL "sos lib include directories")
set(SOS_LIB_CONFIG sys CACHE INTERNAL "sos build config release")
set(SOS_LIB_ARCH armv7-m CACHE INTERNAL "sos build ")
set(SOS_LIB_SOURCELIST ${SYS_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
set(SOS_LIB_ARCH armv7e-m CACHE INTERNAL "sos build ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

set(SOS_LIB_CONFIG sys_debug CACHE INTERNAL "sos build config release")
set(SOS_LIB_ARCH armv7-m CACHE INTERNAL "sos build ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
set(SOS_LIB_ARCH armv7e-m CACHE INTERNAL "sos build ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

set(SOS_LIB_INCLUDE_DIRECTORIES src include include/posix CACHE INTERNAL "sos lib include directories")
set(SOS_LIB_CONFIG crt CACHE INTERNAL "sos build config release")
set(SOS_LIB_ARCH armv7-m CACHE INTERNAL "sos build ")
set(SOS_LIB_SOURCELIST ${CRT_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)
set(SOS_LIB_ARCH armv7e-m CACHE INTERNAL "sos build ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

install(DIRECTORY include/cortexm include/device include/mcu include/sos include/usbd DESTINATION include)
install(DIRECTORY include/posix/ DESTINATION include)
install(DIRECTORY ldscript/ DESTINATION lib/ldscripts)
install(DIRECTORY cmake/ DESTINATION cmake)
