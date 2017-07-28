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

#ifndef DEVICE_LED_PWM_H_
#define DEVICE_LED_PWM_H_

#include "mcu/pwm.h"
#include "sos/dev/led.h"
#include "sos/fs/devfs.h"


enum {
	LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH = (1<<0),
};

typedef struct MCU_PACK {
	pwm_config_t pwm;
	u32 loc; //location of the channel
	u32 o_flags;
} led_pwm_config_t;

int led_pwm_open(const devfs_handle_t * cfg);
int led_pwm_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int led_pwm_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int led_pwm_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int led_pwm_close(const devfs_handle_t * cfg);


#endif /* DEVICE_LED_PWM_H_ */
