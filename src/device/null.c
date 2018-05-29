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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include <errno.h>
#include <stddef.h>
#include "device/null.h"


int null_open(const devfs_handle_t * handle){
	return 0;
}

int null_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    return SYSFS_SET_RETURN(ENOTSUP);
}

int null_read(const devfs_handle_t * handle, devfs_async_t * async){
    return -1; //EOF
}

int null_write(const devfs_handle_t * handle, devfs_async_t * async){
    //accept the data
    return async->nbyte;
}

int null_close(const devfs_handle_t * handle){
	return 0;
}

