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
#include "mcu/pio.h"
#include "device/led.h"

#include "mcu/debug.h"

static void led_setattr(const devfs_handle_t * handle, const led_attr_t * attr);

int led_open(const devfs_handle_t * handle){
	return 0;
}

int led_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	led_info_t * info = ctl;
	switch(request){
	case I_LED_GETINFO:
		info->o_flags = LED_FLAG_ENABLE | LED_FLAG_DISABLE | LED_FLAG_INIT;
		info->o_events = 0;
		break;

	case I_LED_SETATTR:
		led_setattr(handle, ctl);
		break;

	case I_LED_SETACTION:
		errno = ENOTSUP;
		return -1;

	default:
		errno = EINVAL;
		return -1;

	}

	return 0;
}

int led_read(const devfs_handle_t * handle, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int led_write(const devfs_handle_t * handle, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int led_close(const devfs_handle_t * handle){
	return 0;
}

void led_setattr(const devfs_handle_t * handle, const led_attr_t * attr){
	const led_config_t * config = handle->config;
	u32 o_flags = attr->o_flags;

	if( o_flags & LED_FLAG_INIT ){
		//initialize the pin
		pio_attr_t pio_attr;
		pio_attr.o_flags = PIO_FLAG_SET_OUTPUT;
		pio_attr.o_pinmask = (1<<config->pin);
		mcu_pio_setattr(handle, &pio_attr);
	}

	if( o_flags & LED_FLAG_ENABLE ){

		if( config->o_flags & LED_CONFIG_FLAG_IS_ACTIVE_HIGH ){
			mcu_pio_setmask(handle, (void*)(1<<config->pin));
		} else {
			mcu_pio_clrmask(handle, (void*)(1<<config->pin));
		}

	} else if( o_flags & LED_FLAG_DISABLE ){

		if( config->o_flags & LED_CONFIG_FLAG_IS_ACTIVE_HIGH ){
			mcu_pio_clrmask(handle, (void*)(1<<config->pin));
		} else {
			mcu_pio_setmask(handle, (void*)(1<<config->pin));
		}
	}

	//set brightness is not supported by this driver

}


