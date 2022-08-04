
if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
  list(APPEND SOURCELIST ${SRC_SOURCES_PREFIX}/config.h)
	cmsdk_add_subdirectory(SOURCELIST sys)
else()
	cmsdk_add_subdirectory(SOURCELIST link_transport)
	cmsdk_add_subdirectory(SOURCELIST link)
endif()

set(SOURCES
	${SOURCELIST}
	sos_config_template.h
	PARENT_SCOPE)
