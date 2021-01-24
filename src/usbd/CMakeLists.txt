

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
	set(SOURCES
		usbd_control.c
		usbd_standard.c
		sos_link_transport_usb_link_descriptors.c
		sos_link_transport_usb_link_vcp_descriptors.c
		sos_link_transport_usb_vcp_descriptors.c
		sos_link_transport_usb_dual_vcp_descriptors.c
		sos_link_transport_usb.c
		PARENT_SCOPE)
endif()
