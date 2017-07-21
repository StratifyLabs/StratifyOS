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

/*! \ingroup SCHED
 * @{
 *
 */

/*! \file */


#include "sched_flags.h"

void sched_debug_print_tasks(){
	int i;
	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) ){
			mcu_debug("Task %d Details (0x%X):\n", i, task_table[i].flags);
			mcu_debug("\tMem Addr: 0x%X\n", (unsigned int)sos_sched_table[i].attr.stackaddr);
			mcu_debug("\tCurrent Stack Ptr:  0x%X\n", (unsigned int)task_table[i].sp);
			mcu_debug("\tStack Size: %d\n", sos_sched_table[i].attr.stacksize);
			mcu_debug("\tSched Priority %d\n", sos_sched_table[i].attr.schedparam.sched_priority);
		} else {
			mcu_debug("Task %d is not enabled\n", i);
		}
	}
}

void sched_debug_print_active(){
	int i;
	mcu_debug("Active Tasks:\n");
	for(i=1; i < task_get_total(); i++){
		if ( sched_active_asserted(i) ){
			mcu_debug("\t%d\n", i);
		}
	}
}

void sched_debug_print_executing(){
	int i;
	mcu_debug("Executing Tasks:\n");
	for(i=0; i < task_get_total(); i++){
		if ( task_exec_asserted(i) ){
			mcu_debug("\t%d\n", i);
		}
	}
}


void sched_debug_print_timers(){
	int i;
	uint32_t timer[2];
	mcu_debug("Task Timers:\n");
	for(i=0; i < task_get_total(); i++){
		if ( task_enabled(i) ){
			task_get_timer(timer, i);
			mcu_debug("\t%d 0x%08X 0x%08X\n", i, (unsigned int)timer[1], (unsigned int)timer[0]);
		}
	}
}



/*! @} */

