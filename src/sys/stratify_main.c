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
#include "mcu/cortexm.h"

#include "sched/sched_flags.h"

static void init_hw();

int _main() MCU_WEAK;
int _main(){
	init_hw();

	if ( sched_init() < 0 ){ //Initialize the data used for the scheduler
		_mcu_cortexm_priv_disable_interrupts(NULL);
		while(1){ mcu_board_event(MCU_BOARD_CONFIG_EVENT_PRIV_FATAL, 0); }
	}

	if ( sched_start(stratify_board_config.start, 10) < 0 ){
		_mcu_cortexm_priv_disable_interrupts(NULL);
		while(1){ mcu_board_event(MCU_BOARD_CONFIG_EVENT_PRIV_FATAL, 0); }
	}

	_mcu_cortexm_priv_disable_interrupts(NULL);
	while(1){ mcu_board_event(MCU_BOARD_CONFIG_EVENT_PRIV_FATAL, 0); }
	return 0;
}


/*! \details This function initializes the hardware
 *
 */
void init_hw(){
	_mcu_core_initclock(1);
	mcu_fault_init();
	_mcu_cortexm_priv_enable_interrupts(NULL); //Enable the interrupts
}

int kernel_request(int request, void * data){
	return 0;
}


/*! @} */
