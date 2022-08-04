
if (${CMSDK_BUILD_CONFIG} STREQUAL arm)
  set(SOURCELIST
    auth_pure_code.c)
endif ()

set(SOURCES ${SOURCELIST} PARENT_SCOPE)
