/* Copyright 2011-2019 Tyler Gilbert;
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

/*! \addtogroup LTDC_DEV LTDC
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_LTDC_H_
#define _MCU_LTDC_H_


#include "sos/dev/ltdc.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 status;
	u32 rising;
	u32 falling;
} ltdc_event_t;

typedef struct MCU_PACK {
	ltdc_attr_t attr; //default attributes
} ltdc_config_t;

int mcu_ltdc_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
int mcu_ltdc_read(const devfs_handle_t * handle, devfs_async_t * async) MCU_ROOT_CODE;
int mcu_ltdc_write(const devfs_handle_t * handle, devfs_async_t * async) MCU_ROOT_CODE;
int mcu_ltdc_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int mcu_ltdc_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_ltdc_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_ltdc_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_ltdc_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;


#ifdef __cplusplus
}
#endif

#endif /* _MCU_LTDC_H_ */

/*! @} */

