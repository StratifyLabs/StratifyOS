


cmsdk_add_subdirectory(SOURCELIST cortexm)
cmsdk_add_subdirectory(SOURCELIST device)
cmsdk_add_subdirectory(SOURCELIST mcu)
cmsdk_add_subdirectory(SOURCELIST posix)
cmsdk_add_subdirectory(SOURCELIST sos)
cmsdk_add_subdirectory(SOURCELIST usbd)

set(SOURCES ${SOURCELIST} PARENT_SCOPE)
