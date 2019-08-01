/* Copyright 2011-2018 Tyler Gilbert; 
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
#include "config.h"
#include "mcu/mcu.h"
#include "cortexm/cortexm.h"

#include "scheduler/scheduler_local.h"

static void init_hw();

int _main() MCU_WEAK;
int _main(){
	init_hw();

	if ( scheduler_init() < 0 ){ //Initialize the data used for the scheduler
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "main sched init");
		cortexm_disable_interrupts();
		while(1){}
	}

	if ( scheduler_start(sos_board_config.start) < 0 ){
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "main sched start");
		cortexm_disable_interrupts();
		while(1){}
	}

	mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "main sched failed");
	cortexm_disable_interrupts();
	while(1){}
	return 0;
}


/*! \details This function initializes the hardware
 *
 */
void init_hw(){
	if( mcu_core_initclock(1) < 0 ){
		while(1){}
	}
	mcu_fault_init();
	cortexm_enable_interrupts(); //Enable the interrupts

#if 0
	if ( mcu_debug_init() < 0 ){
		cortexm_disable_interrupts();
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, (void*)"dbgi");
	}

	mcu_debug_log_info(MCU_DEBUG_SYS, "MCU Debug start--");
#endif
}

int kernel_request(int request, void * data){
	errno = ENOSYS;
	return -1;
}

const void * kernel_request_api(u32 request){
	return 0;
}

