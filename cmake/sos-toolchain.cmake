
if( CMAKE_BINARY_DIR MATCHES ".*_link" )
	set(BUILD_CONFIG link CACHE INTERNAL "sos build config is link")
elseif( CMAKE_BINARY_DIR MATCHES ".*_arm" )
	set(BUILD_CONFIG arm CACHE INTERNAL "sos build config is arm")
else()
	message( FATAL_ERROR "No Configuration available build in *_link or *_arm directory")
endif()

message(STATUS "Using SDK PATH: ${SOS_SDK_PATH}")

if(${BUILD_CONFIG} STREQUAL "arm")
	include(sos-gcc-toolchain)
elseif()
	include(link-toolchain)
endif()




