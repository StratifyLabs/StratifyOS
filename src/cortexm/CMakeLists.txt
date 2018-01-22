
if( ${SOS_BUILD_CONFIG} STREQUAL arm )
    set(SOURCES ${SOURCES}
      ${SOURCES_PREFIX}/cortexm_local.h
      ${SOURCES_PREFIX}/cortexm.c
      ${SOURCES_PREFIX}/fault.c
      ${SOURCES_PREFIX}/mpu.c
      ${SOURCES_PREFIX}/task_mpu.c
      ${SOURCES_PREFIX}/task_process.c
      ${SOURCES_PREFIX}/task.c
      ${SOURCES_PREFIX}/task_local.h
      PARENT_SCOPE)
endif()
