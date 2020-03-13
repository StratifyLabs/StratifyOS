

if( ${SOS_BUILD_CONFIG} STREQUAL link )
		set(SOURCES
			link_bootloader.c
			link_debug.c
			link_dir.c
			link_file.c
			link_phy.c
			link_process.c
			link_stdio.c
			link_sys_attr.c
			link_time.c
			link.c
			link_local.h
      PARENT_SCOPE)
  endif()
