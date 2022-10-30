cmake_minimum_required (VERSION 3.12)
cmsdk2_minimum_required(2.1.2)

project(StratifyOS
	LANGUAGES CXX C ASM
	VERSION 4.3.1)

set(${PROJECT_NAME}_VERSION ${PROJECT_VERSION} CACHE INTERNAL "StratifyOS project version")

include( ${CMAKE_CURRENT_SOURCE_DIR}/${CMSDK_BUILD_CONFIG}.cmake )

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/StratifyOS.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/StratifyOS_crt.cmake
	DESTINATION ${CMSDK_LOCAL_PATH}/cmake/targets)


