

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	set(SOURCES ${SOURCES}
		${SOURCES_PREFIX}/link1_transport.c
		${SOURCES_PREFIX}/link2_transport.c
		${SOURCES_PREFIX}/link_transport_slave.c
		${SOURCES_PREFIX}/link1_transport_slave.c
		${SOURCES_PREFIX}/link2_transport_slave.c
		PARENT_SCOPE)
endif()

if( ${SOS_BUILD_CONFIG} STREQUAL link )
	set(SOURCES
		${SOURCES_PREFIX}/link_transport_master.c
		${SOURCES_PREFIX}/link1_transport.c
		${SOURCES_PREFIX}/link1_transport_master.c
		${SOURCES_PREFIX}/link2_transport.c
		${SOURCES_PREFIX}/link2_transport_master.c
		PARENT_SCOPE)
endif()
