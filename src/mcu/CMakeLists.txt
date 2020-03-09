

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
		set(SOURCES
			mcu_debug.c
			mcu_sync_io.c
			mcu.c
			PARENT_SCOPE)
  endif()
