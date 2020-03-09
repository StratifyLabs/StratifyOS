
if( ${SOS_BUILD_CONFIG} STREQUAL arm )
		set(SOURCES
			cortexm_local.h
			cortexm.c
			fault.c
			mpu.c
			task_mpu.c
			task_process.c
			task.c
			task_local.h
      PARENT_SCOPE)
endif()
