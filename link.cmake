

#Add sources to the project
set(SOURCES_PREFIX ${CMAKE_SOURCE_DIR}/src CACHE INTERNAL "src prefix")
add_subdirectory(src)
list(APPEND SOS_LIB_SOURCELIST ${SOURCES})

set(SOS_LIB_CONFIG link CACHE INTERNAL "sos build config release")
set(SOS_LIB_ARCH link CACHE INTERNAL "sos build ")
include(${SOS_TOOLCHAIN_CMAKE_PATH}/sos-lib.cmake)

install(FILES include/mcu/types.h DESTINATION include/mcu)
install(FILES include/mcu/mcu.h DESTINATION include/mcu)
install(DIRECTORY include/sos DESTINATION include)
