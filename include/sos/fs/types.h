/* Copyright 2011-2018 Tyler Gilbert;
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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef SOS_FS_TYPES_H_
#define SOS_FS_TYPES_H_

#include "mcu/types.h"

typedef open_file_t sysfs_file_t;

enum sysfs_user {
	SYSFS_ROOT = 0,
	SYSFS_USER = 1,
	SYSFS_GROUP = 0
};

typedef struct {
	u32 port /*! The port associated with the device (for mcu peripherals) */;
	const void * config /*! Pointer to device configuration (flash) */;
	void * state /*! \brief Pointer to device state (RAM) */;
} devfs_handle_t;


typedef struct {
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

typedef struct {
	devfs_open_t open;
	devfs_ioctl_t ioctl;
	devfs_read_t read;
	devfs_write_t write;
	devfs_close_t close;
} devfs_driver_t;


typedef struct {
    devfs_async_t * read; //used with read operations
    devfs_async_t * write; //used with write operations
} devfs_transfer_handler_t;

//mcu_execute_read_complete(devfs_transfer_handler_t * transfer_handler);

typedef struct {
	char name[NAME_MAX] /*! The name of the device */;
	u16 uid /*! The user ID of the device (either user or root) */;
	u16 mode /*! The file access values */;
	devfs_driver_t driver /*! \brief The driver functions */;
	devfs_handle_t handle /*! \brief The configuration for the device */;
} devfs_device_t;


#define DEVFS_IOC_IDENT_CHAR 'D'

typedef char devfs_get_name_t[NAME_MAX];

#define I_DEVFS_GETNAME _IOCTLW(DEVFS_IOC_IDENT_CHAR, I_MCU_TOTAL, devfs_get_name_t)



#endif /* SOS_FS_TYPES_H_ */
