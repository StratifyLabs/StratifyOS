

if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
		set(SOURCES
			boot_config.h
			boot_debug.c
			boot_link_transport_usb.c
			boot_link.c
			boot_main.c
			boot_interrupt_handlers.c
			../sys/sos_led_root.c
			PARENT_SCOPE)
  endif()
