

# Globs
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/include/*)
file(GLOB_RECURSE CMAKE_FILES ${CMAKE_SOURCE_DIR}/cmake/*)

#Add sources to the project
sos_sdk_add_subdirectory(SOS_SOURCELIST src)
list(APPEND SOS_SOURCELIST ${HEADERS} ${CMAKE_FILES})

set(SOS_CONFIG release)
set(SOS_OPTION link)
set(SOS_ARCH link)
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

install(FILES include/mcu/types.h DESTINATION include/mcu)
install(FILES include/mcu/mcu.h DESTINATION include/mcu)
install(DIRECTORY include/sos DESTINATION include)

option(SOS_SKIP_CMAKE "Dont install the cmake files" OFF)

if(NOT SOS_SKIP_CMAKE)
	install(DIRECTORY cmake/ DESTINATION arm-none-eabi/cmake)
endif()
