
if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  list(APPEND SOURCELIST ${SRC_SOURCES_PREFIX}/config.h)
	sos_sdk_add_subdirectory(SOURCELIST sys)
else()
	sos_sdk_add_subdirectory(SOURCELIST link_transport)
	sos_sdk_add_subdirectory(SOURCELIST link)
endif()

set(SOURCES
	${SOURCELIST}
	sos_config_template.h
	PARENT_SCOPE)
