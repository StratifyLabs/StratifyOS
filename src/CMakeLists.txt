
if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  list(APPEND SOURCELIST ${SRC_SOURCES_PREFIX}/config.h)
	sos_sdk_add_subdirectory(SOURCELIST sys)
	sos_sdk_add_subdirectory(SOURCELIST usbd)
else()
	sos_sdk_add_subdirectory(SOURCELIST link_transport)
	sos_sdk_add_subdirectory(SOURCELIST link)
endif()

set(SOURCES ${SOURCELIST} PARENT_SCOPE)
