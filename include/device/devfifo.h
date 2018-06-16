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


#ifndef DEV_DEVFIFO_H_
#define DEV_DEVFIFO_H_

#include "sos/dev/devfifo.h"
#include "sos/fs/devfs.h"

/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
	int head;
	int tail;
	bool overflow;
	devfs_async_t * rop;
	int len;
} devfifo_state_t;

int devfifo_open(const devfs_handle_t * handle);
int devfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int devfifo_read(const devfs_handle_t * handle, devfs_async_t * rop);
int devfifo_write(const devfs_handle_t * handle, devfs_async_t * wop);
int devfifo_close(const devfs_handle_t * handle);


#endif /* DEV_DEVFIFO_H_ */

