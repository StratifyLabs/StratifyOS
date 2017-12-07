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

/*! \addtogroup I2S Inter-Integated Sound (I2S) Master/Slave Driver
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module implements an I2S driver for hardware which supports I2S.
 *
 *
 */

/*! \file
 * \brief I2S Header File
 *
 */

#ifndef I2S_H_
#define I2S_H_

#include "sos/dev/i2s.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	i2s_attr_t attr; //default attributes
} i2s_config_t;

typedef struct MCU_PACK {
	u32 value;
} i2s_event_t;

int mcu_i2s_open(const devfs_handle_t * cfg) MCU_ROOT_CODE;
int mcu_i2s_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_i2s_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_i2s_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_ROOT_CODE;
int mcu_i2s_close(const devfs_handle_t * cfg) MCU_ROOT_CODE;

int mcu_i2s_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_i2s_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_i2s_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_i2s_mute(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_i2s_unmute(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;


#ifdef __cplusplus
}
#endif


#endif /* I2S_H_ */

/*! @} */
