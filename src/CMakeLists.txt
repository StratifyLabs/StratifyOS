

set(SRC_SOURCES_PREFIX ${SOURCES_PREFIX})

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/boot CACHE INTERNAL "src prefix")
  add_subdirectory(boot)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/cortexm CACHE INTERNAL "src prefix")
  add_subdirectory(cortexm)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/device CACHE INTERNAL "src prefix")
  add_subdirectory(device)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/link_common CACHE INTERNAL "src prefix")
  add_subdirectory(link_common)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/mcu CACHE INTERNAL "src prefix")
  add_subdirectory(mcu)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/sys CACHE INTERNAL "src prefix")
  add_subdirectory(sys)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/usbd CACHE INTERNAL "src prefix")
  add_subdirectory(usbd)
  list(APPEND SOURCELIST ${SOURCES})

else()

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/link CACHE INTERNAL "src prefix")
  add_subdirectory(link)
  list(APPEND SOURCELIST ${SOURCES})

  set(SOURCES_PREFIX ${SRC_SOURCES_PREFIX}/link_common CACHE INTERNAL "src prefix")
  add_subdirectory(link_common)
  list(APPEND SOURCELIST ${SOURCES})

endif()

set(SOURCES ${SOURCELIST} PARENT_SCOPE)
