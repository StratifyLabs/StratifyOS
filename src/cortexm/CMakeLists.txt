
if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
		set(SOURCES
			cortexm_local.h
			cortexm.c
			auth.c
			devfs.c
			fault.c
			util.c
			fault_handler.c
			fault_local.h
			mpu.c
			task_mpu.c
			task_process.c
			task.c
			task_local.h
      PARENT_SCOPE)
endif()
