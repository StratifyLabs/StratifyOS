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

#include <stratify/stratify.h>
#include "config.h"
#include "mcu/mcu.h"
#include "mcu/core.h"

#include "sched/sched_flags.h"

extern void gled_priv_on(void * args);
extern void gled_priv_off(void * args);
void gled_priv_error(void * args) MCU_PRIV_EXEC_CODE;
static void init_hw(void);
static void check_reset_source(void);

/*! \details This function runs the operating system.
 *
 */

int _main(void) MCU_WEAK;
int _main(void){
	init_hw();


	if ( sched_start(initial_thread, 10) < 0 ){
		_mcu_core_priv_disable_interrupts(NULL);
		gled_priv_error(0);
	}

	_mcu_core_priv_disable_interrupts(NULL);
	gled_priv_error(0);
	while(1);
	return 0;
}


/*! \details This function initializes the hardware
 *
 */
void init_hw(void){
	_mcu_core_initclock(1);
	mcu_fault_init();
	_mcu_core_priv_enable_interrupts(NULL); //Enable the interrupts

	if ( sched_init() < 0 ){ //Initialize the hardware used for the scheduler
		_mcu_core_priv_disable_interrupts(NULL);
		gled_priv_error(0);
	}

	check_reset_source();
}

void check_reset_source(void){
	//fault_t fault;
	core_attr_t attr;

	mcu_core_getattr(0, &attr);

	switch(attr.reset_type){
	case CORE_RESET_SRC_WDT:
		//log the reset
		break;
	case CORE_RESET_SRC_POR:
	case CORE_RESET_SRC_BOR:
	case CORE_RESET_SRC_EXTERNAL:
		//read and discard the fault
		//mcu_fault_load(&fault);
		break;
	}
}


void gled_priv_error(void * args){
	while(1){
		gled_priv_on(0);
		_mcu_core_delay_ms(50);
		gled_priv_off(0);
		_mcu_core_delay_ms(50);
	}
}

int kernel_request(int request, void * data){
	return 0;
}



/*! @} */
