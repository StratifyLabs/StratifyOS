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

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

//#include "config.h"
#include <errno.h>
#include <signal.h>
#include "cortexm/mpu.h"
#include "mcu/mcu.h"
#include "mcu/core.h"
#include "sos/sos.h"

#include "scheduler_local.h"
#include "../unistd/unistd_local.h"
#include "../signal/sig_local.h"

#include "mcu/debug.h"

extern void mcu_core_pendsv_handler();

extern void sos_root_trace_event(void * info);

void mcu_fault_event_handler(fault_t * fault){
	int i;
	int pid;

	pid = task_get_pid( task_get_current() );

	if ( m_scheduler_fault.fault.num == 0 ){
		m_scheduler_fault.tid = task_get_current();
		m_scheduler_fault.pid = pid;
		memcpy((void*)&m_scheduler_fault.fault, fault, sizeof(fault_t));

		//grab stack and memory usage
		u32 tid = m_scheduler_fault.tid;
		u32 tid_thread_zero = task_get_thread_zero(pid);
		u32 end_of_heap = scheduler_calculate_heap_end( tid_thread_zero );

		volatile void * stack;

		//free heap size is thread 0 stack location - end of the heap

		if( tid == tid_thread_zero ){
			cortexm_get_thread_stack_ptr((void**)&stack);
		} else {
			stack = sos_task_table[tid_thread_zero].sp;
		}

		m_scheduler_fault.free_heap_size =
				(u32)stack -
				end_of_heap;

		if( task_thread_asserted(tid) ){
			//since this is a thread the stack is on the heap (malloc'd)
			cortexm_get_thread_stack_ptr((void**)&stack);
			m_scheduler_fault.free_stack_size =
					(u32)stack -
					(u32)sos_sched_table[tid].attr.stackaddr -
					SCHED_DEFAULT_STACKGUARD_SIZE;
		} else {
			//free stack is the same as the free heap for first thread in process
			m_scheduler_fault.free_stack_size =
					m_scheduler_fault.free_heap_size;
		}

	}

	if ( (pid == 0) || (task_enabled_active_not_stopped( task_get_current() ) == 0) ){
		mcu_fault_save(fault); //save the fault in NV memory then log it to the filesystem on startup
		//The OS has experienced a fault

#if MCU_DEBUG
		char buffer[128];
		scheduler_fault_build_string(buffer, "\n");
		mcu_debug_root_write_uart(buffer, strnlen(buffer,128));
#endif

		if( sos_board_config.trace_event != 0 ){
			link_trace_event_t event;
			event.posix_trace_event.posix_event_id = 0;
			event.posix_trace_event.posix_pid = pid;
			event.posix_trace_event.posix_thread_id = task_get_current();
			event.posix_trace_event.posix_timestamp_tv_sec = 0;
			event.posix_trace_event.posix_timestamp_tv_nsec = 0;
			scheduler_fault_build_trace_string((char*)event.posix_trace_event.data);
			sos_board_config.trace_event(&event);
		}

#if MCU_DEBUG
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, buffer);
#else
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "OS FAULT");
#endif

	} else {

#if defined MCU_DEBUG
		char buffer[128];
		scheduler_fault_build_string(buffer, 0);
		mcu_debug_log_error(MCU_DEBUG_SYS, "Task Fault:%d:%s", task_get_current(), buffer);
		//check for a stack overflow error
		u32 psp;
		cortexm_get_thread_stack_ptr((void**)&psp);
		if( psp <= (u32)sos_task_table[task_get_current()].mem.stackguard.address + sos_task_table[task_get_current()].mem.stackguard.size ){
			mcu_debug_log_error(MCU_DEBUG_SYS, "Stack Overflow");
		}
#endif
		//send a signal to kill the task
		for(i=1; i < task_get_total(); i++){
			if ( task_get_pid(i) == pid ){
				//stop running the task
				task_root_delete(i);
#if 0 //this was the old way but could cause an infinite fault loop Issue #163
				if( task_thread_asserted(i) == 0 ){
					//reset the stack of the processes main task
					task_root_resetstack(i);
					//send the kill signal
					if( signal_root_send(0, i, SIGKILL, 0, 0, 0) < 0 ){
						//kill manually -- for example, if the target task doesn't have enough memory to accept SIGKILL
						task_root_delete(i);
						scheduler_root_update_on_sleep();
					}
					break;
				}
#endif

			}
		}
		scheduler_root_update_on_sleep();

		/* scheduler_root_update_on_sleep() sets the PEND SV interrupt.
		 *
		 *
		 * However, the pendsv handler won't be executed until this interrupt returns.
		 * When this interrupt returns it will restore the hw frame to the PSP. If the fault
		 * was caused be a bad PSP value, another fault will immediately be triggered. The hw
		 * frame is restored in un-privileged mode.
		 *
		 * By calling mcu_core_pendsv_handler() manually, the context will be changed to a
		 * non-faulty thread. This means the faulting PSP will never be touched again.
		 *
		 * The mcu_core_pendsv_handler() will save the context of the faulty PSP, but the PSP
		 * was reset above so, it won't overflow to memory owned by another process
		 *
		 *
		 */
		task_root_resetstack(task_get_current()); //this will make sure the hardware has stack space to shutdown on a stack overflow error
		mcu_core_pendsv_handler();
	}
}

/*! @} */
