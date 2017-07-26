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


int null_open(const devfs_handle_t * cfg){
	return 0;
}

int null_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return 0;
}

int null_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return -1;
}

int null_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return wop->nbyte;
}

int null_close(const devfs_handle_t * cfg){
	return 0;
}

