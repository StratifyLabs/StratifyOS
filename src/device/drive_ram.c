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


#include "sos/sos.h"

#include <errno.h>
#include <unistd.h>
#include "device/drive_ram.h"
#include "cortexm/cortexm.h"
#include "mcu/debug.h"

#include "mcu/core.h"
#include "cortexm/task.h"

static int calculate_transaction_size(const devfs_handle_t * handle, devfs_async_t * async){
	const drive_ram_config_t * config = handle->config;
	//check for overflow
	int size = async->nbyte;
	if( async->loc + size > config->size ){
		size = config->size - async->loc;
		if( size <= 0 ){
			return SYSFS_SET_RETURN(EINVAL);
		}
	}
	return size;
}

int drive_ram_open(const devfs_handle_t * handle){
	return 0;
}

int drive_ram_read(const devfs_handle_t * handle, devfs_async_t * async){
	const drive_ram_config_t * config = handle->config;
	int size = calculate_transaction_size(handle, async);
	if( size > 0 ){
		memcpy(async->buf, config->memory + async->loc, size);
	}
	return size;
}


int drive_ram_write(const devfs_handle_t * handle, devfs_async_t * async){
	const drive_ram_config_t * config = handle->config;
	int size = calculate_transaction_size(handle, async);
	if( size > 0 ){
		memcpy(config->memory + async->loc, async->buf, size);
	}
	return size;
}

int drive_ram_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	const drive_ram_config_t * config = handle->config;
	drive_info_t * info = ctl;
	drive_attr_t * attr = ctl;
	u32 o_flags;

	switch(request){
		case I_DRIVE_SETATTR:
			o_flags = attr->o_flags;
			if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){
				int size = attr->end - attr->start + 1;
				if( size > 0 ){
					memset(config->memory + attr->start, 0xff, size);
				}
				return size;
			}

			if( o_flags & DRIVE_FLAG_INIT ){
				return 0;
			}

			if( o_flags & DRIVE_FLAG_RESET ){
				return 0;
			}

			if( o_flags & DRIVE_FLAG_ERASE_DEVICE ){
				memset(config->memory, 0xff, config->size);
				return 0;
			}

			break;

		case I_DRIVE_ISBUSY:
			return 0;

		case I_DRIVE_GETINFO:
			info->o_flags = DRIVE_FLAG_INIT |
					DRIVE_FLAG_ERASE_BLOCKS |
					DRIVE_FLAG_ERASE_DEVICE |
					DRIVE_FLAG_RESET;
			info->o_events = 0;
			info->addressable_size = 1;
			info->bitrate = 1;
			info->erase_block_size = 0;
			info->erase_block_time = 0;
			info->erase_device_time = 0;
			info->num_write_blocks = config->size;
			info->write_block_size = 1;
			break;

		default:
			return SYSFS_SET_RETURN(EINVAL);
	}
	return 0;
}

int drive_ram_close(const devfs_handle_t * handle){
	return 0;
}





/*! @} */

