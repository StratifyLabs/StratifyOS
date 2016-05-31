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

/*! \addtogroup STFY
 * @{
 */

/*! \file */

#include "stratify/stratify.h"
#include "config.h"
#include "mcu/mcu.h"
#include "mcu/core.h"

#include "sched/sched_flags.h"

static void init_hw();

int _main() MCU_WEAK;
int _main(){
	init_hw();

	if ( sched_init() < 0 ){ //Initialize the data used for the scheduler
		_mcu_core_priv_disable_interrupts(NULL);
		while(1){ mcu_event(MCU_BOARD_CONFIG_EVENT_PRIV_ERROR, 0); }
	}

	if ( sched_start(stratify_board_config.start, 10) < 0 ){
		_mcu_core_priv_disable_interrupts(NULL);
		while(1){ mcu_event(MCU_BOARD_CONFIG_EVENT_PRIV_ERROR, 0); }
	}

	_mcu_core_priv_disable_interrupts(NULL);
	while(1){ mcu_event(MCU_BOARD_CONFIG_EVENT_PRIV_ERROR, 0); }
	return 0;
}


/*! \details This function initializes the hardware
 *
 */
void init_hw(){
	_mcu_core_initclock(1);
	mcu_fault_init();
	_mcu_core_priv_enable_interrupts(NULL); //Enable the interrupts
}

int kernel_request(int request, void * data){
	return 0;
}

void stratify_led_startup(){
	int i;
	int duty;
	const int factor = 30;
	duty = 0;
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

void stratify_led_priv_on(void * args){
	pio_attr_t attr;
	attr.mask = (1<<mcu_board_config.led.pin);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(mcu_board_config.led.port, &attr);
	if( mcu_board_config.flags & MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH ){
		mcu_pio_setmask(mcu_board_config.led.port, (void*)(1<<mcu_board_config.led.pin));
	} else {
		mcu_pio_clrmask(mcu_board_config.led.port, (void*)(1<<mcu_board_config.led.pin));
	}
}



void stratify_led_priv_off(void * args){
	pio_attr_t attr;
	attr.mode = PIO_MODE_INPUT | PIO_MODE_DIRONLY;
	attr.mask = (1<<mcu_board_config.led.pin);
	if( mcu_pio_setattr(mcu_board_config.led.port, &attr) < 0 ){
		mcu_debug("failed to setattr\n");
	}
}

void stratify_led_priv_error(void * args){
	while(1){
		stratify_led_priv_on(0);
		_mcu_core_delay_ms(50);
		stratify_led_priv_off(0);
		_mcu_core_delay_ms(50);
	}
}



/*! @} */
