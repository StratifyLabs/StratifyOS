

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
    set(SOURCES ${SOURCES}
      ${SOURCES_PREFIX}/mcu_debug.c
      ${SOURCES_PREFIX}/mcu_sync_io.c
			${SOURCES_PREFIX}/mcu.c
			PARENT_SCOPE)
  endif()
