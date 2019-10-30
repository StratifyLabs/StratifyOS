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
#include "device/drive_sdio.h"
#include "cortexm/cortexm.h"
#include "mcu/debug.h"

#include "mcu/core.h"
#include "cortexm/task.h"


int drive_sdio_open(const devfs_handle_t * handle){
    return mcu_sdio_open(handle);
}

int drive_sdio_read(const devfs_handle_t * handle, devfs_async_t * async){
    return mcu_sdio_read(handle, async);
}


int drive_sdio_write(const devfs_handle_t * handle, devfs_async_t * async){
    return mcu_sdio_write(handle, async);
}

int drive_sdio_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    drive_info_t * info = ctl;
    drive_attr_t * attr = ctl;
    u32 o_flags;
    sdio_info_t sdio_info;
    sdio_info_t sdio_attr;
    int result;

    switch(request){
    case I_DRIVE_SETATTR:
        o_flags = attr->o_flags;
        if( o_flags & (DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_ERASE_DEVICE) ){

            if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){

                sdio_attr_t sdio_attr;
                sdio_attr.o_flags = SDIO_FLAG_ERASE_BLOCKS;
                sdio_attr.start = attr->start;
                sdio_attr.end = attr->end;
					 int result = mcu_sdio_setattr(handle, &sdio_attr);
					 if( result < 0 ){
						 return result;
					 }
					 return attr->end - attr->start;
            }
        }

        if( o_flags & DRIVE_FLAG_INIT ){

            //this will init the SD card with the default settings
            return mcu_sdio_setattr(handle, 0);
        }

        break;

    case I_DRIVE_ISBUSY:
        sdio_attr.o_flags = SDIO_FLAG_GET_CARD_STATE;
        result = mcu_sdio_setattr(handle, &sdio_attr);

        if( result < 0 ){ return result; }

        return (result != SDIO_CARD_STATE_TRANSFER);

    case I_DRIVE_GETINFO:
        result = mcu_sdio_getinfo(handle, &sdio_info);
        if( result < 0 ){ return result; }

        info->o_flags = DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_INIT;
        info->o_events = sdio_info.o_events;
		  info->addressable_size = sdio_info.block_size;
        info->bitrate = sdio_info.freq;
        info->erase_block_size = sdio_info.block_size;
        info->erase_block_time = 0;
        info->erase_device_time = 0;
        info->num_write_blocks = sdio_info.block_count;
        info->write_block_size = sdio_info.block_size;
		  info->page_program_size = sdio_info.block_size;
        break;

    default:
        return mcu_sdio_ioctl(handle, request, ctl);
    }
    return 0;
}

int drive_sdio_close(const devfs_handle_t * handle){
    return mcu_sdio_close(handle);
}





/*! @} */

