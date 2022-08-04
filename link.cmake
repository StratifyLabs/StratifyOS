
# Globs - these don't affect the build
file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*)

#Add sources to the project
cmsdk_add_subdirectory(SOS_INTERFACE_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/include)
cmsdk_add_subdirectory(SOS_SOURCELIST ${CMAKE_CURRENT_SOURCE_DIR}/src)


set(CMSDK_ARCH link)

cmsdk_library_target(BUILD_RELEASE StratifyOS "" release link)

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
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	)


cmsdk_library_target(BUILD_DEBUG StratifyOS "" debug link)
add_library(${BUILD_DEBUG_TARGET} STATIC)
cmsdk_copy_target(${BUILD_RELEASE_TARGET} ${BUILD_DEBUG_TARGET})

cmsdk_library("${BUILD_RELEASE_OPTIONS}")
cmsdk_library("${BUILD_DEBUG_OPTIONS}")

install(FILES include/mcu/mcu.h DESTINATION include/StratifyOS/mcu)
install(DIRECTORY include/sos DESTINATION include/StratifyOS PATTERN CMakelists.txt EXCLUDE)


