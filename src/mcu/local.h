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


#ifndef MCU_LOCAL_H_
#define MCU_LOCAL_H_

#include <errno.h>
#include "mcu/debug.h"
#include "device/sys.h"
#include "sos/fs/devfs.h"



static inline int mcu_open(const devfs_handle_t * handle,
		int (*powered_on)(const devfs_handle_t*),
		void (*power_on)(const devfs_handle_t*)
) MCU_ALWAYS_INLINE MCU_ROOT_CODE;
int mcu_open(const devfs_handle_t * handle,
		int (*powered_on)(const devfs_handle_t*),
		void (*power_on)(const devfs_handle_t*)
){

	//Turn the device on
	power_on(handle);
	return 0;
}

static inline int mcu_ioctl(const devfs_handle_t * handle,
		int request,
		void * ctl,
		int (*powered_on)(const devfs_handle_t*),
		int (* const ioctl_func_table[])(const devfs_handle_t*, void*),
		const int ioctl_func_table_size) MCU_ALWAYS_INLINE MCU_ROOT_CODE;
int mcu_ioctl(const devfs_handle_t * handle,
		int request,
		void * ctl,
		int (*powered_on)(const devfs_handle_t*),
		int (* const ioctl_func_table[])(const devfs_handle_t*, void*),
		const int ioctl_func_table_size){
	uint32_t periph_request;


	if ( !powered_on(handle) ){
		errno = EIO;
		return -200;
	}

	periph_request = _IOCTL_NUM(request);

	if ( periph_request < ioctl_func_table_size ) {
		return ioctl_func_table[periph_request](handle, ctl);
	}

	errno = EINVAL;
	return -300;
}

static inline int mcu_read(const devfs_handle_t * handle,
		devfs_async_t * rop,
		int (*powered_on)(const devfs_handle_t*),
		int (*read)(const devfs_handle_t*, devfs_async_t*) ) MCU_ALWAYS_INLINE MCU_ROOT_CODE;
int mcu_read(const devfs_handle_t * handle,
		devfs_async_t * rop,
		int (*powered_on)(const devfs_handle_t*),
		int (*read)(const devfs_handle_t*, devfs_async_t*) ){

	//Make sure the device is powered up
	if ( !powered_on(handle) ){
		errno = EIO;
		return -1;
	}

	return read(handle, rop);
}

static inline int mcu_write(const devfs_handle_t * handle,
		devfs_async_t * wop,
		int (*powered_on)(const devfs_handle_t*),
		int (*write)(const devfs_handle_t*, devfs_async_t*) ) MCU_ALWAYS_INLINE MCU_ROOT_CODE;
int mcu_write(const devfs_handle_t* handle,
		devfs_async_t * wop,
		int (*powered_on)(const devfs_handle_t*),
		int (*write)(const devfs_handle_t*, devfs_async_t*) ){

	//Make sure the device is powered up
	if ( !powered_on(handle) ){
		errno = EIO;
		return -1;
	}

	return write(handle, wop);
}



static inline int mcu_close(const devfs_handle_t * handle,
		int (*powered_on)(const devfs_handle_t*),
		void (*power_off)(const devfs_handle_t*) ) MCU_ALWAYS_INLINE MCU_ROOT_CODE;
int mcu_close(const devfs_handle_t * handle,
		int (*powered_on)(const devfs_handle_t*),
		void (*power_off)(const devfs_handle_t*) ){
	power_off(handle);
	return 0;
}

#endif

