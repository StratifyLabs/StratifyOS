

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
		set(SOURCES
			usbd_control.c
			usbd_standard.c
      PARENT_SCOPE)
  endif()
