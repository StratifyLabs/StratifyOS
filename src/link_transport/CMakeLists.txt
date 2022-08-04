

if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
	set(SOURCES
		link1_transport.c
		link2_transport.c
		link3_transport.c
		link_transport_slave.c
		link1_transport_slave.c
		link2_transport_slave.c
		link3_transport_slave.c
		PARENT_SCOPE)
endif()

if( ${CMSDK_BUILD_CONFIG} STREQUAL link )
	set(SOURCES
		link_transport_master.c
		link1_transport.c
		link1_transport_master.c
		link2_transport.c
		link2_transport_master.c
		link3_transport.c
		link3_transport_master.c
		PARENT_SCOPE)
endif()
