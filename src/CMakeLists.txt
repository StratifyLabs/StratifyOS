

set(SRC_SOURCES_PREFIX ${SOURCES_PREFIX})

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/cortexm)
  add_subdirectory(cortexm)
  list(APPEND SOURCELIST ${SOURCES})

  list(APPEND SOURCELIST ${SRC_SOURCES_PREFIX}/config.h)

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/device)
  add_subdirectory(device)
  list(APPEND SOURCELIST ${SOURCES})

	set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/link_transport)
	add_subdirectory(link_transport)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/mcu)
  add_subdirectory(mcu)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/sys)
  add_subdirectory(sys)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/usbd)
  add_subdirectory(usbd)
  list(APPEND SOURCELIST ${SOURCES})

else()

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/link)
  add_subdirectory(link)
  list(APPEND SOURCELIST ${SOURCES})

	set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/link_transport)
	add_subdirectory(link_transport)
  list(APPEND SOURCELIST ${SOURCES})

endif()

set(SOURCES ${SOURCELIST} PARENT_SCOPE)
