

if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
	set(SOURCES
		aio/aio.c
		crt/crt_sys.c
		dirent/dirent.c
		link/link_thread.c
		malloc/_calloc.c
		malloc/_realloc.c
		malloc/_sbrk.c
		malloc/calloc.c
		malloc/mallinfo.c
		malloc/malloc_stats.c
		malloc/malloc.c
		malloc/malloc_local.h
		malloc/mallocr.c
		malloc/mlock.c
		malloc/realloc.c
		mqueue/mqueue.c
		process/_system.c
		process/install.c
		process/launch.c
		process/process_start.c
		process/process_start.h
		pthread/pthread_attr_init.c
		pthread/pthread_attr.c
		pthread/pthread_cancel.c
		pthread/pthread_cleanup.c
		pthread/pthread_cond.c
		pthread/pthread_condattr.c
		pthread/pthread_create.c
		pthread/pthread_mutex_init.c
		pthread/pthread_mutex.c
		pthread/pthread_mutex_local.h
		pthread/pthread_mutexattr_init.c
		pthread/pthread_mutexattr.c
		pthread/pthread_rwlock.c
		pthread/pthread_rwlockattr.c
		pthread/pthread_schedparam.c
		pthread/pthread_self.c
		sched/sched.c
		scheduler/scheduler_debug.c
		scheduler/scheduler_fault.c
		scheduler/scheduler_fault.h
		scheduler/scheduler_flags.h
		scheduler/scheduler_init.c
		scheduler/scheduler_process.c
		scheduler/scheduler_root.c
		scheduler/scheduler_root.h
		scheduler/scheduler_thread.c
		#scheduler/scheduler_tmr.c
		scheduler/scheduler_timing.c
		scheduler/scheduler_timing.h
		scheduler/scheduler.c
		scheduler/scheduler_local.h
		semaphore/sem.c
		signal/_kill.c
		signal/_wait.c
		signal/pthread_kill.c
		signal/sig_realtime.c
		signal/sig_local.h
		signal/signal_handler.c
		signal/signal.c
		signal/sigset.c
		socket/socket_api.c
		sysfs/appfs_local.h
		sysfs/appfs_ram.c
		sysfs/appfs_util.c
		sysfs/appfs_mem_dev.c
		sysfs/appfs.c
		sysfs/assetfs.c
		sysfs/drive_assetfs.c
		sysfs/devfs_aio.c
		sysfs/devfs_data_transfer.c
		sysfs/devfs.c
		sysfs/devfs_local.h
		sysfs/rootfs.c
		sysfs/sysfs_file.c
		sysfs/sysfs.c
		termios/termios.c
		time/_gettimeofday.c
		time/_itimer.c
		time/timer.c
		time/_settimeofday.c
		time/_times.c
		time/clocks.c
		time/hibernate.c
		trace/posix_trace_attr.c
		trace/posix_trace.c
		trace/sos_trace.c
		unistd/_close.c
		unistd/_execve.c
		unistd/_exit.c
		unistd/_fcntl.c
		unistd/_fstat.c
		unistd/_getpid.c
		unistd/_getppid.c
		unistd/_isatty.c
		unistd/_link.c
		unistd/_lseek.c
		unistd/_open.c
		unistd/_read.c
		unistd/_rename.c
		unistd/_stat.c
		unistd/_symlink.c
		unistd/_unlink.c
		unistd/_write.c
		unistd/access.c
		unistd/crypt.c
		unistd/chmod.c
		unistd/chown.c
		unistd/fsync.c
		unistd/ioctl.c
		unistd/lstat.c
		unistd/mkdir.c
		unistd/rmdir.c
		unistd/sleep.c
		unistd/uidgid.c
		unistd/usleep.c
		unistd/unistd_fs.h
		unistd/unistd_local.h
		assert_func.c
		libgcc.c
		check_config.h
		sos_default_thread.c
		sos_led.c
		sos_led_root.c
		sos_main.c
		sos_debug.c
		sos_interrupt_handlers.c
		symbols.c
		sys_dev.c
		auth/auth_dev.c
		auth/device_aes_api.c
		auth/device_random_api.c
		auth/random_root_api.c
		PARENT_SCOPE)
endif()
