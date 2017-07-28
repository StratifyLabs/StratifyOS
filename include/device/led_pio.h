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


#ifndef MCU_LED_H_
#define MCU_LED_H_

#include "sos/dev/led.h"
#include "sos/fs/devfs.h"


enum {
	LED_PIO_CONFIG_FLAG_IS_ACTIVE_HIGH = (1<<0),
};

typedef struct MCU_PACK {
	u32 o_flags;
	u32 pin;
} led_pio_config_t;

int led_pio_open(const devfs_handle_t * cfg);
int led_pio_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int led_pio_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int led_pio_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int led_pio_close(const devfs_handle_t * cfg);


#endif /* MCU_LED_H_ */
