

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
    set(SOURCES ${SOURCES}
      ${SOURCES_PREFIX}/usbd_control.c
      ${SOURCES_PREFIX}/usbd_standard.c
      PARENT_SCOPE)
  endif()
