cmake_minimum_required (VERSION 3.12)

set(SOS_NAME sos)
set(SOS_PROJECT_VERSION 4.2.0 CACHE INTERNAL "Stratify OS version")

project(StratifyOS
	LANGUAGES CXX C ASM
	VERSION ${SOS_PROJECT_VERSION})

include( ${CMAKE_CURRENT_SOURCE_DIR}/${CMSDK_BUILD_CONFIG}.cmake )

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/StratifyOS.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/StratifyOS_crt.cmake
	DESTINATION ${CMSDK_LOCAL_PATH}/cmake/targets)


