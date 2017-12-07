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

/*! \addtogroup PWM_DEV PWM
 * @{
 * \ingroup DEV
 *
 */

#ifndef _MCU_PWM_H_
#define _MCU_PWM_H_

#include "sos/dev/pwm.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 value;
} pwm_event_data_t;

typedef struct MCU_PACK {
	pwm_attr_t attr; //default attributes
} pwm_config_t;

int mcu_pwm_open(const devfs_handle_t * cfg) MCU_ROOT_CODE;
int mcu_pwm_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_pwm_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_pwm_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_close(const devfs_handle_t * cfg) MCU_ROOT_CODE;

int mcu_pwm_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_setchannel(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_getchannel(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_set(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_get(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_enable(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_pwm_disable(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_PWM_H_ */

/*! @} */
