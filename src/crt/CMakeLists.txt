

if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
	set(SOURCES
		crt_common.c
		crt_common.h
		crt_symbols.S
		crt.c
		operator.cpp
		PARENT_SCOPE)
endif()
