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

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

//#include "config.h"
#include <errno.h>
#include <signal.h>
#include "mcu/mcu.h"
#include "mcu/core.h"
#include "stratify/stratify.h"

#include "sched_flags.h"
#include "../unistd/unistd_flags.h"
#include "../signal/sig_local.h"

#include "mcu/debug.h"

extern void stratify_priv_trace_event(void * info);

void mcu_fault_event_handler(fault_t * fault){
#if SINGLE_TASK == 0
	int i;
	int pid;

	pid = task_get_pid( task_get_current() );

	if ( sched_fault.fault.num == 0 ){
		sched_fault.tid = task_get_current();
		sched_fault.pid = pid;
		memcpy((void*)&sched_fault.fault, fault, sizeof(fault_t));
	}

	if ( pid == 0 ){
		mcu_fault_save(fault); //save the fault in NV memory then log it to the filesystem on startup
		//The OS has experienced a fault

#if MCU_DEBUG
		sched_fault_build_string(mcu_debug_buffer);
		mcu_priv_write_debug_uart(NULL);
		_mcu_core_delay_ms(200);
#endif

		if( stratify_board_config.notify_write != 0 ){
			link_posix_trace_event_info_t info;
			info.posix_event_id = 0;
			info.posix_pid = pid;
			info.posix_thread_id = task_get_current();
			info.posix_timestamp_tv_sec = 0;
			info.posix_timestamp_tv_nsec = 0;
			sched_fault_build_trace_string((char*)info.data);
			stratify_priv_trace_event(&info);
		}

		mcu_board_event(MCU_BOARD_CONFIG_EVENT_PRIV_FATAL, 0);

	} else {

		//send a signal to kill the task

		for(i=1; i < task_get_total(); i++){
			if ( task_get_pid(i) == pid ){

				if( task_isthread_asserted(i) == 0 ){
					//reset the stack of the processes main task
					task_priv_resetstack(i);
					//send the kill signal
					if( signal_priv_send(0, i, SIGKILL, 0, 0, 0) < 0 ){
						//kill manually -- for example, if the target task doesn't have enough memory to accept SIGKILL
						task_priv_del(i);
						sched_priv_update_on_sleep();
					}
					break;
				}

			}
		}

	}
#else


	while(1);
#endif
}

/*! @} */
