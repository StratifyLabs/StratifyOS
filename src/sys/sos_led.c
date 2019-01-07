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

void sos_led_startup(){
	int i;
	int duty;
	const int factor = 10;
	duty = 0;
	if( mcu_board_config.led.port != 255 ){
		for(i=0; i < 100; i++){
			duty = i*factor;
			cortexm_svcall(sos_led_root_enable, 0);
			usleep(duty);
			cortexm_svcall(sos_led_root_disable, 0);
			usleep(100*factor - duty);
		}

		for(i=0; i < 100; i++){
			duty = i*factor;
			cortexm_svcall(sos_led_root_enable, 0);
			usleep(100*factor - duty);
			cortexm_svcall(sos_led_root_disable, 0);
			usleep(duty);
		}
	}
}


