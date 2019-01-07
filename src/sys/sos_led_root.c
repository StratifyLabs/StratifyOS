/* Copyright 2011-2017 Tyler Gilbert;
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
#include "mcu/core.h"
#include "mcu/pio.h"
#include "cortexm/cortexm.h"

void sos_led_root_enable(void * args){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		devfs_handle_t handle;
		handle.port = mcu_board_config.led.port;
		attr.o_pinmask = (1<<mcu_board_config.led.pin);
		attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
		mcu_pio_setattr(&handle, &attr);
		if( mcu_board_config.o_flags & MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH ){
			mcu_pio_setmask(&handle, (void*)(1<<mcu_board_config.led.pin));
		} else {
			mcu_pio_clrmask(&handle, (void*)(1<<mcu_board_config.led.pin));
		}
	}
}

void sos_led_root_disable(void * args){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		devfs_handle_t handle;
		handle.port = mcu_board_config.led.port;
		attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_DIRONLY;
		attr.o_pinmask = (1<<mcu_board_config.led.pin);
		mcu_pio_setattr(&handle, &attr);
	}
}

void sos_led_root_error(void * args){
	while(1){
		sos_led_root_enable(0);
		cortexm_delay_ms(50);
		sos_led_root_disable(0);
		cortexm_delay_ms(50);
	}
}


