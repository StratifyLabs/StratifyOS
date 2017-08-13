/* Copyright 2011-2016 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SOS_FS_DEVFS_H_
#define SOS_FS_DEVFS_H_

#include <sys/dirent.h>
#include "mcu/types.h"

#define DEVFS_GET_PORT(x) (x->port)

#if defined __cplusplus
extern "C" {
#endif


/*! \details This structure is for executing a function as a result of an interrupt
 * in non-privileged mode.
 */
typedef struct {
	int tid;
	int si_signo;
	int si_sigcode;
	int sig_value;
	int keep;
} devfs_signal_callback_t;


/*! \details This function can be set as the callback for mcu_action_t.
 * In this case, the context is pointing to a devfs_signal_callback_t.  When
 * the event happens, it will send a signal to the specified task.
 *
 * If keep is non-zero, the signal will be sent each time the interrupt happens.
 * Otherwise, the signal is just sent one time.
 *
 * In the example below, the thread below will receive a SIGUSR1 the next time
 * the external interrupt goes low.  The run_on_sigusr1() will execute when the
 * signal is received.
 *
 * \code
 * #include <pthread.h>
 * #include <signal.h>
 * #include <stfy/Hal.hpp>
 *
 * void run_on_sigusr1(int a){
 * 	printf("got sigusr1\n");
 * }
 *
 * Eint intr(0);
 * intr.init();
 *
 * signal(SIGUSR1, (_sig_func_ptr)run_on_sigusr1);
 *
 * devfs_signal_callback_t sig; //this must be valid when the interrupt happens
 * sig.tid = pthread_self();
 * sig.signo = SIGUSR1;
 * sig.keep = 0;
 * intr.setaction(0, EINT_ACTION_EVENT_FALLING, signal_callback, &sig);
 * \endcode
 *
 */
int devfs_signal_callback(void * context, const mcu_event_t * data);


typedef struct MCU_PACK {
	u32 port /*! The port associated with the device (for mcu peripherals) */;
	const void * config /*! Pointer to device configuration (flash) */;
	void * state /*! \brief Pointer to device state (RAM) */;
} devfs_handle_t;


typedef struct MCU_PACK {
	int tid /*! The calling task ID */;
	int flags /*! The flags for the open file descriptor */;
	int loc /*! The location to read or write */;
	union {
		const void * buf_const /*! Pointer to const void buffer */;
		void * buf /*! Pointer to void buffer */;
	};
	int nbyte /*! The number of bytes to transfer */;
	mcu_event_handler_t handler /*! The function to call when the operation completes */;
} devfs_async_t;

typedef devfs_async_t device_3_transfer_t;

typedef int (*devfs_open_t)(const devfs_handle_t*);
typedef int (*devfs_ioctl_t)(const devfs_handle_t*, int, void*);
typedef int (*devfs_read_t)(const devfs_handle_t*, devfs_async_t *);
typedef int (*devfs_write_t)(const devfs_handle_t*, devfs_async_t *);
typedef int (*devfs_close_t)(const devfs_handle_t*);

typedef struct MCU_PACK {
	devfs_open_t open;
	devfs_ioctl_t ioctl;
	devfs_read_t read;
	devfs_write_t write;
	devfs_close_t close;
} devfs_driver_t;



typedef struct {
	char name[NAME_MAX] /*! The name of the device */;
	u16 uid /*! The user ID of the device (either user or root) */;
	u16 mode /*! The file access values */;
	devfs_driver_t driver /*! \brief The driver functions */;
	devfs_handle_t handle /*! \brief The configuration for the device */;
} devfs_device_t;


extern const devfs_device_t devfs_list[];

const devfs_handle_t * devfs_lookup_handle(const devfs_device_t * list, const char * name);


#define DEVFS_MODE(mode_value, uid_value, type) .mode = mode_value | type, \
		.uid = uid_value

#define DEVFS_DRIVER(driver_name) .driver.open = driver_name##_open, \
		.driver.close = driver_name##_close, \
		.driver.ioctl = driver_name##_ioctl, \
		.driver.read = driver_name##_read, \
		.driver.write = driver_name##_write



#define DEVFS_DEVICE(device_name, periph_name, handle_port, handle_config, handle_state, mode_value, uid_value, device_type) { \
		.name = device_name, \
		DEVFS_MODE(mode_value, uid_value, device_type), \
		DEVFS_DRIVER(periph_name), \
		.handle.port = handle_port, \
		.handle.state = handle_state, \
		.handle.config = handle_config \
}

#define DEVFS_TERMINATOR { \
		.driver.open = NULL \
}


static inline bool devfs_is_terminator(const devfs_device_t * dev);
bool devfs_is_terminator(const devfs_device_t * dev){
	if ( dev->driver.open == NULL ){
		return true;
	}
	return false;
}



int devfs_init(const void * cfg);
int devfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode);
int devfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte);
int devfs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte);
int devfs_read_async(const void * cfg, void * handle, devfs_async_t * op);
int devfs_write_async(const void * cfg, void * handle, devfs_async_t * op);
int devfs_ioctl(const void * cfg, void * handle, int request, void * ctl);
int devfs_close(const void * cfg, void ** handle);
int devfs_fstat(const void * cfg, void * handle, struct stat * st);
int devfs_stat(const void * cfg, const char * path, struct stat * st);
int devfs_opendir(const void * cfg, void ** handle, const char * path);
int devfs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry);
int devfs_closedir(const void * cfg, void ** handle);

#define DEVFS_MOUNT(mount_loc_name, cfgp, access_mode) { \
		.mount_path = mount_loc_name, \
		.access = access_mode, \
		.mount = devfs_init, \
		.unmount = SYSFS_NOTSUP, \
		.ismounted = sysfs_always_mounted, \
		.startup = SYSFS_NOTSUP, \
		.mkfs = SYSFS_NOTSUP, \
		.open = devfs_open, \
		.read_async = devfs_read_async, \
		.write_async = devfs_write_async, \
		.ioctl = devfs_ioctl, \
		.read = devfs_read, \
		.write = devfs_write, \
		.close = devfs_close, \
		.rename = SYSFS_NOTSUP, \
		.unlink = SYSFS_NOTSUP, \
		.mkdir = SYSFS_NOTSUP, \
		.rmdir = SYSFS_NOTSUP, \
		.remove = SYSFS_NOTSUP, \
		.opendir = devfs_opendir, \
		.closedir = devfs_closedir, \
		.readdir_r = devfs_readdir_r, \
		.link = SYSFS_NOTSUP, \
		.symlink = SYSFS_NOTSUP, \
		.stat = devfs_stat, \
		.lstat = SYSFS_NOTSUP, \
		.fstat = devfs_fstat, \
		.chmod = SYSFS_NOTSUP, \
		.chown = SYSFS_NOTSUP, \
		.unlock = SYSFS_NOTSUP_VOID, \
		.config = cfgp, \
}

#if defined __cplusplus
}
#endif


#endif /* SOS_FS_DEVFS_H_ */
