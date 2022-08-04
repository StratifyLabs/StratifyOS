

if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
		set(SOURCES
			mcu_sync_io.c
			mcu.c
			PARENT_SCOPE)
  endif()
