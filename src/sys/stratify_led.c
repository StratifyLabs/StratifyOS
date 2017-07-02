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

#include "stratify/stratify.h"
#include "mcu/core.h"
#include "mcu/pio.h"

void stratify_led_startup(){
	int i;
	int duty;
	const int factor = 10;
	duty = 0;
	if( mcu_board_config.led.port != 255 ){
		for(i=0; i < 100; i++){
			duty = i*factor;
			mcu_core_privcall(stratify_led_priv_on, 0);
			usleep(duty);
			mcu_core_privcall(stratify_led_priv_off, 0);
			usleep(100*factor - duty);
		}

		for(i=0; i < 100; i++){
			duty = i*factor;
			mcu_core_privcall(stratify_led_priv_on, 0);
			usleep(100*factor - duty);
			mcu_core_privcall(stratify_led_priv_off, 0);
			usleep(duty);
		}
	}
}

void stratify_led_priv_on(void * args){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		attr.mask = (1<<mcu_board_config.led.pin);
		attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
		mcu_pio_setattr(mcu_board_config.led.port, &attr);
		if( mcu_board_config.o_flags & MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH ){
			mcu_pio_setmask(mcu_board_config.led.port, (void*)(1<<mcu_board_config.led.pin));
		} else {
			mcu_pio_clrmask(mcu_board_config.led.port, (void*)(1<<mcu_board_config.led.pin));
		}
	}
}

void stratify_led_priv_off(void * args){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		attr.mode = PIO_MODE_INPUT | PIO_MODE_DIRONLY;
		attr.mask = (1<<mcu_board_config.led.pin);
		mcu_pio_setattr(mcu_board_config.led.port, &attr);
	}
}

void stratify_led_priv_error(void * args){
	while(1){
		stratify_led_priv_on(0);
		_mcu_cortexm_delay_ms(50);
		stratify_led_priv_off(0);
		_mcu_cortexm_delay_ms(50);
	}
}


