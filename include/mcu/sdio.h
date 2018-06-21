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

/*! \addtogroup SD_DEV SDIO
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_SDIO_H_
#define _MCU_SDIO_H_

#include "sos/dev/sdio.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 value;
} sdio_event_data_t;

typedef struct MCU_PACK {
    sdio_attr_t attr; //default attributes
} sdio_config_t;


int mcu_sdio_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
int mcu_sdio_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_sdio_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_sdio_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int mcu_sdio_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_sdio_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_sdio_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_sdio_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_sdio_getcid(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_sdio_getcsd(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_sdio_getstatus(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;



#ifdef __cplusplus
}
#endif

#endif /* _MCU_SDIO_H_ */

/*! @} */
