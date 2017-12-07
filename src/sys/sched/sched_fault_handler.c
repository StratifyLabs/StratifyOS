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
#include "sos/sos.h"

#include "sched_local.h"
#include "../unistd/unistd_local.h"
#include "../signal/sig_local.h"

#include "mcu/debug.h"

extern void sos_priv_trace_event(void * info);

void mcu_fault_event_handler(fault_t * fault){
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
		char buffer[128];
		sched_fault_build_string(buffer);
		mcu_debug_root_write_uart(buffer, strnlen(buffer,128));
#endif

		if( sos_board_config.trace_event != 0 ){
			link_trace_event_t event;
			event.posix_trace_event.posix_event_id = 0;
			event.posix_trace_event.posix_pid = pid;
			event.posix_trace_event.posix_thread_id = task_get_current();
			event.posix_trace_event.posix_timestamp_tv_sec = 0;
			event.posix_trace_event.posix_timestamp_tv_nsec = 0;
			sched_fault_build_trace_string((char*)event.posix_trace_event.data);
			sos_board_config.trace_event(&event);
		}

#if MCU_DEBUG
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, buffer);
#else
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "OS FAULT");
#endif

	} else {

		//send a signal to kill the task

		for(i=1; i < task_get_total(); i++){
			if ( task_get_pid(i) == pid ){

				if( task_isthread_asserted(i) == 0 ){
					//reset the stack of the processes main task
					task_root_resetstack(i);
					//send the kill signal
					if( signal_priv_send(0, i, SIGKILL, 0, 0, 0) < 0 ){
						//kill manually -- for example, if the target task doesn't have enough memory to accept SIGKILL
						task_root_del(i);
						sched_priv_update_on_sleep();
					}
					break;
				}

			}
		}

	}
}

/*! @} */
