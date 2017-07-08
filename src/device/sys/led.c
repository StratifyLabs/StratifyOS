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
#include "mcu/sys.h"
#include "sos/dev/sys.h"

#include "mcu/debug.h"

/*

int led_open(const devfs_handle_t * cfg){
	return 0;
}

int led_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	led_req_t * req = ctl;
	mcu_pin_t port_pin;
	pio_attr_t req_attr;
	if( request == I_LED_SET ){
		if ( req->channel > 3 ){
			errno = EINVAL;
			return -1;
		}

		port_pin = cfg->pcfg.pio[req->channel];

		if ( req->on != 0 ){
			req_attr.o_pinmask = (1<<port_pin.pin);
			req_attr.mode = PIO_FLAG_SET_OUTPUT;
			mcu_pio_setattr(port_pin.port, &req_attr);
			if ( cfg->pin_assign == LED_ACTIVE_LOW ){
				mcu_pio_clrmask(port_pin.port, (void*)(1<<port_pin.pin));
			} else {
				mcu_pio_setmask(port_pin.port, (void*)(1<<port_pin.pin));
			}
		} else {
			req_attr.o_pinmask = (1<<port_pin.pin);
			req_attr.mode = PIO_FLAG_SET_INPUT;
			mcu_pio_setattr(port_pin.port, &req_attr);
			if ( cfg->pin_assign == LED_ACTIVE_LOW ){
				mcu_pio_setmask(port_pin.port, (void*)(1<<port_pin.pin));
			} else {
				mcu_pio_clrmask(port_pin.port, (void*)(1<<port_pin.pin));
			}
		}

	}
	return 0;
}

int led_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int led_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int led_close(const devfs_handle_t * cfg){
	return 0;
}
*/

