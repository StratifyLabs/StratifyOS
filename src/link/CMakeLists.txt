

if( ${SOS_BUILD_CONFIG} STREQUAL link )
    set(SOURCES ${SOURCES}
      ${SOURCES_PREFIX}/link_bootloader.c
      ${SOURCES_PREFIX}/link_debug.c
      ${SOURCES_PREFIX}/link_dir.c
      ${SOURCES_PREFIX}/link_file.c
      ${SOURCES_PREFIX}/link_phy.c
      ${SOURCES_PREFIX}/link_process.c
      ${SOURCES_PREFIX}/link_stdio.c
      ${SOURCES_PREFIX}/link_sys_attr.c
      ${SOURCES_PREFIX}/link_time.c
      ${SOURCES_PREFIX}/link.c
      ${SOURCES_PREFIX}/link_local.h
      PARENT_SCOPE)
  endif()
