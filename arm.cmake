

#check for LWIP
include(CheckIncludeFiles)
check_include_files("lwip/sockets.h" HAVE_LWIP_SOCKETS_H)
if(HAVE_LWIP_SOCKETS_H)
  set(SOS_LIB_DEFINITIONS HAVE_LWIP_SOCKETS_H=1)
endif()

#Add sys sources
file(GLOB CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*)
file(GLOB LDSCRIPT_SOURCES ${CMAKE_SOURCE_DIR}/ldscript/*)
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/* )
set(SYS_SOURCELIST ${SOURCES} ${CMAKE_SOURCES} ${LDSCRIPT_SOURCES} ${HEADER_FILES})

#Add sys sources
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src/crt)
add_subdirectory(src/crt)
set(CRT_SOURCELIST ${SOURCES} ${HEADER_FILES})

#Add sys sources
set(SOURCES "")
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src/boot)
add_subdirectory(src/boot)
set(BOOT_SOURCELIST ${SOURCES} ${HEADER_FILES})

set(SOS_LIB_INCLUDE_DIRECTORIES src include include/posix)
set(SOS_LIB_OPTION sys)
set(SOS_LIB_TYPE release)
set(SOS_LIB_BUILD_FLAGS "")
set(SOS_LIB_SOURCELIST ${SYS_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)


set(SOS_LIB_OPTION sys)
set(SOS_LIB_TYPE debug)
set(SOS_LIB_BUILD_FLAGS "")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)

set(SOS_LIB_OPTION crt)
set(SOS_LIB_TYPE release)
set(SOS_LIB_BUILD_FLAGS -mlong-calls)
set(SOS_LIB_SOURCELIST ${CRT_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)

set(SOS_LIB_OPTION boot)
set(SOS_LIB_TYPE release)
set(SOS_LIB_BUILD_FLAGS "")
set(SOS_LIB_SOURCELIST ${BOOT_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)

set(SOS_LIB_TYPE debug)
set(SOS_LIB_SOURCELIST ${BOOT_SOURCELIST})
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib-std.cmake)

install(DIRECTORY include/cortexm include/device include/mcu include/sos include/usbd DESTINATION include)
install(DIRECTORY include/posix/ DESTINATION include)
install(DIRECTORY ldscript/ DESTINATION lib/ldscripts)
install(DIRECTORY cmake/ DESTINATION cmake)
