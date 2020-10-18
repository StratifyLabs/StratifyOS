
# Globs - these don't affect the build
file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*)

#Add sources to the project
sos_sdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_SOURCE_DIR}/include)
sos_sdk_add_subdirectory(SOS_SOURCELIST ${CMAKE_SOURCE_DIR}/src)


set(SOS_ARCH link)

sos_sdk_library_target(BUILD_RELEASE StratifyOS "" release link)

add_library(${BUILD_RELEASE_TARGET} STATIC)
target_sources(${BUILD_RELEASE_TARGET}
	PRIVATE
	${SOS_INTERFACE_SOURCELIST}
	${SOS_SOURCELIST}
	${CMAKE_SOURCES}
	)

target_include_directories(${BUILD_RELEASE_TARGET}
	PUBLIC
	$<INSTALL_INTERFACE:include/StratifyOS>
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	)


sos_sdk_library_target(BUILD_DEBUG StratifyOS "" debug link)
add_library(${BUILD_DEBUG_TARGET} STATIC)
sos_sdk_copy_target(${BUILD_RELEASE_TARGET} ${BUILD_DEBUG_TARGET})

sos_sdk_library_target(BUILD_COVERAGE StratifyOS "" coverage link)
add_library(${BUILD_COVERAGE_TARGET} STATIC)
sos_sdk_copy_target(${BUILD_RELEASE_TARGET} ${BUILD_COVERAGE_TARGET})


target_compile_options(${BUILD_COVERAGE_TARGET}
	PUBLIC
	--coverage
	)

sos_sdk_library("${BUILD_RELEASE_OPTIONS}")
sos_sdk_library("${BUILD_COVERAGE_OPTIONS}")
sos_sdk_library("${BUILD_DEBUG_OPTIONS}")

install(FILES include/mcu/types.h DESTINATION include/StratifyOS/mcu)
install(FILES include/mcu/mcu.h DESTINATION include/StratifyOS/mcu)
install(DIRECTORY include/sos DESTINATION include/StratifyOS PATTERN CMakelists.txt EXCLUDE)

option(SOS_SKIP_CMAKE "Don't install the cmake files" OFF)

if(NOT SOS_SKIP_CMAKE)
	install(DIRECTORY cmake/ DESTINATION cmake)
endif()
