

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
    set(SOURCES ${SOURCES}
      ${SOURCES_PREFIX}/crt_common.c
      ${SOURCES_PREFIX}/crt_symbols.S
      ${SOURCES_PREFIX}/crt.c
      ${SOURCES_PREFIX}/operator.cpp
      PARENT_SCOPE)
  endif()
