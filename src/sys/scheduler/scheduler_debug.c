// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \ingroup SCHED
 * @{
 *
 */

/*! \file */


#include "scheduler_local.h"

void scheduler_debug_print_tasks(){
	int i;
	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) ){
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Task %d Details (0x%X):", i, sos_task_table[i].flags);
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\tMem Addr: 0x%X", (unsigned int)sos_sched_table[i].attr.stackaddr);
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\tCurrent Stack Ptr:  0x%X", (unsigned int)sos_task_table[i].sp);
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\tStack Size: %d", sos_sched_table[i].attr.stacksize);
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\tSched Priority %d", sos_sched_table[i].attr.schedparam.sched_priority);
		} else {
            sos_debug_log_error(SOS_DEBUG_SCHEDULER, "Task %d is not enabled", i);
		}
	}
}

void scheduler_debug_print_active(){
	int i;
    sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Active Tasks:");
	for(i=1; i < task_get_total(); i++){
		if ( task_active_asserted(i) ){
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\t%d", i);
		}
	}
}

void scheduler_debug_print_executing(){
	int i;
    sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Executing Tasks:");
	for(i=0; i < task_get_total(); i++){
		if ( task_exec_asserted(i) ){
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\t%d", i);
		}
	}
}


void scheduler_debug_print_timers(){
	int i;
	uint32_t timer[2];
    sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Task Timers:");
	for(i=0; i < task_get_total(); i++){
		if ( task_enabled(i) ){
			task_get_timer(timer, i);
            sos_debug_log_info(SOS_DEBUG_SCHEDULER, "\t%d 0x%08X 0x%08X", i, (unsigned int)timer[1], (unsigned int)timer[0]);
		}
	}
}



/*! @} */

