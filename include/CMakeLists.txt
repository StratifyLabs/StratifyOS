


sos_sdk_add_subdirectory(SOURCELIST cortexm)
sos_sdk_add_subdirectory(SOURCELIST device)
sos_sdk_add_subdirectory(SOURCELIST mcu)
sos_sdk_add_subdirectory(SOURCELIST posix)
sos_sdk_add_subdirectory(SOURCELIST sos)
sos_sdk_add_subdirectory(SOURCELIST usbd)

set(SOURCES ${SOURCELIST} PARENT_SCOPE)
