
if( CMAKE_BINARY_DIR MATCHES ".*_link" )
	set(SOS_TOOLCHAIN_BUILD_CONFIG link CACHE INTERNAL "sos build config is link")
elseif( CMAKE_BINARY_DIR MATCHES ".*_arm" )
	set(SOS_TOOLCHAIN_BUILD_CONFIG arm CACHE INTERNAL "sos build config is arm")
else()
	message( FATAL_ERROR "No Configuration available build in *_link or *_arm directory")
endif()

string(COMPARE EQUAL ${SOS_TOOLCHAIN_BUILD_CONFIG} arm IS_ARM)
message(STATUS "Toolchain SDK PATH: ${SOS_SDK_PATH} ARM:${IS_ARM}")

if(IS_ARM)
	include(${CMAKE_CURRENT_LIST_DIR}/toolchains/sos-gcc-toolchain.cmake)
else()
	include(${CMAKE_CURRENT_LIST_DIR}/toolchains/link-toolchain.cmake)
endif()




