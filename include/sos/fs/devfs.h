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

#include "mcu/types.h"

#define DEVFS_GET_PORT(x) (x->port)


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


#ifndef __link

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


#define DEVICE_MODE(mode_value, uid_value, type) .mode = mode_value | type, \
		.uid = uid_value

#define DEVICE_DRIVER(driver_name) .driver.open = driver_name##_open, \
		.driver.close = driver_name##_close, \
		.driver.ioctl = driver_name##_ioctl, \
		.driver.read = driver_name##_read, \
		.driver.write = driver_name##_write



#define DEVICE_PERIPH(device_name, periph_name, port_number, state, mode_value, uid_value, gid_value, device_type) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, device_type), \
		DEVICE_DRIVER(periph_name), \
		.handle.port = port_number \
		.handle.state = state \
		.handle.config = 0 \
}

#define DEVICE_TERMINATOR { \
		.driver.open = NULL \
}


static inline bool devfs_is_terminator(const devfs_device_t * dev);
bool devfs_is_terminator(const devfs_device_t * dev){
	if ( dev->driver.open == NULL ){
		return true;
	}
	return false;
}
#endif



#endif /* SOS_FS_DEVFS_H_ */
