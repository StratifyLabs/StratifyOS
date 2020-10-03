
# Globs - these don't affect the build
file(GLOB_RECURSE CMAKE_FILES ${CMAKE_SOURCE_DIR}/cmake/*)

#Add sources to the project
sos_sdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_SOURCE_DIR}/include)
sos_sdk_add_subdirectory(SOS_SOURCELIST ${CMAKE_SOURCE_DIR}/src)

set(SOS_ARCH link)

add_library(StratifyOS_link_release_link STATIC)
target_sources(StratifyOS_link_release_link
	PUBLIC
	${SOS_INTERFACE_SOURCELIST}
	PRIVATE
	${SOS_SOURCELIST}
	)

target_include_directories(StratifyOS_link_release_link
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
	)

sos_sdk_library(StratifyOS link release link)

install(FILES include/mcu/types.h DESTINATION include/mcu)
install(FILES include/mcu/mcu.h DESTINATION include/mcu)
install(DIRECTORY include/sos DESTINATION include PATTERN CMakelists.txt EXCLUDE)

option(SOS_SKIP_CMAKE "Don't install the cmake files" OFF)

if(NOT SOS_SKIP_CMAKE)
	install(DIRECTORY cmake/ DESTINATION arm-none-eabi/cmake)
endif()
