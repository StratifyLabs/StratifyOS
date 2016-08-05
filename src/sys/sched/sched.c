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

#include "config.h"
#include <stdbool.h>
#include <pthread.h>
#include <reent.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <malloc.h>

#include "mcu/wdt.h"
#include "mcu/debug.h"
#include "mcu/rtc.h"

#include "trace.h"

#include "sched_flags.h"
#include "../unistd/unistd_flags.h"

static int start_first_thread();

volatile int8_t sched_current_priority MCU_SYS_MEM;
volatile int8_t sched_status_changed MCU_SYS_MEM;


static void priv_check_sleep_mode(void * args) MCU_PRIV_EXEC_CODE;
static int check_faults();
static void priv_fault_logged(void * args) MCU_PRIV_EXEC_CODE;

int sched_check_tid(int id){
	if( id < task_get_total() ){
		if ( task_enabled(id) ){
			return 0;
		}
	}
	return -1;
}

void set_uart_priority(){
#if MCU_DEBUG
	mcu_action_t action;
	memset(&action, 0, sizeof(action));
	action.prio = 127;
	mcu_uart_setaction(MCU_DEBUG_PORT, &action);
#endif
}

/* \details This function should be called in main() after all hardware
 * and devices have been initialized.
 *
 * The scheduler executes as task 0.  It is run as part of the round robin with all
 * active tasks of the highest priority.
 *
 * When a task calls sleep() or usleep(), it goes inactive.  When a task does
 * a synchronous read() or write() using underlying asynchronous hardware,
 * the task goes inactive until the operation completes.
 *
 * It an active task has a lower priority of another active task, it stays
 * "active" but is not executed until all higher priority tasks go inactive.
 *
 * If a lower priority task is executing and a higher priority task goes "active" (for example,
 * after sleep() or usleep() timer is expired), the lower priority task is pre-empted.
 */
void scheduler(){
	u8 do_sleep;
	sched_priv_assert_status_change();

	//Elevate the priority of the UART so that it can debug in interrupts
	set_uart_priority();

	//This interval needs to be long enough to allow for flash writes
	if( (stratify_board_config.o_sys_flags & SYS_FLAGS_DISABLE_WDT) == 0 ){
		mcu_wdt_init(WDT_MODE_INTERRUPT|WDT_MODE_CLK_SRC_MAIN, SCHED_RR_DURATION * 10 * stratify_board_config.task_total + 5);
	}

	if ( sched_prepare() ){  //this starts memory protection
		mcu_board_event(MCU_BOARD_CONFIG_EVENT_FATAL, (void*)"sprep");
	}

	mcu_debug("Start first thread\n");
	if ( start_first_thread() ){
		sched_debug("Start first thread failed\n");
		mcu_board_event(MCU_BOARD_CONFIG_EVENT_FATAL, (void*)"strt1t");
	}

	while(1){
		mcu_core_privcall(mcu_wdt_priv_reset, NULL);
		check_faults(); //check to see if a fault needs to be logged
		mcu_core_privcall(priv_check_sleep_mode, &do_sleep);

		//Sleep when nothing else is going on
		if ( do_sleep ){
			_mcu_core_sleep(CORE_SLEEP);
		} else {
			//Otherwise switch to the active task
			mcu_core_privcall(task_priv_switch_context, NULL);
		}
	}
}

void priv_fault_logged(void * args){
	sched_fault.fault.num = 0;
}

int check_faults(){
	char buffer[256];

	if ( sched_fault.fault.num != 0 ){
		//Trace the fault

		//log the fault in a new thread

		sched_fault_build_string(buffer);
		posix_trace_event_addr_tid(
				POSIX_TRACE_FATAL,
				buffer, strlen(buffer),
				(uint32_t)sched_fault.fault.pc + 1,
				sched_fault.tid);
		mcu_debug("%s\n", buffer);

		char hex_buffer[9];
		strcpy(buffer, "ADDR 0x");
		htoa(hex_buffer, (uint32_t)sched_fault.fault.addr);
		strcat(buffer, hex_buffer);
		posix_trace_event_addr_tid(
				POSIX_TRACE_MESSAGE,
				buffer, strlen(buffer),
				(uint32_t)sched_fault.fault.pc + 1,
				sched_fault.tid);
		mcu_debug("ADDR 0x%lX %ld\n", (u32)sched_fault.fault.pc + 1, sched_fault.tid);


		strcpy(buffer, "Caller");
		posix_trace_event_addr_tid(
				POSIX_TRACE_MESSAGE,
				buffer, strlen(buffer),
				(uint32_t)sched_fault.fault.caller,
				sched_fault.tid);
		mcu_debug("Caller 0x%lX %ld\n", (u32)sched_fault.fault.caller, sched_fault.tid);


		strcpy(buffer, "ISR PC");
		posix_trace_event_addr_tid(
				POSIX_TRACE_MESSAGE,
				buffer, strlen(buffer),
				(u32)sched_fault.fault.handler_pc + 1,
				sched_fault.tid);
		mcu_debug("ISR PC 0x%lX %ld\n", (u32)sched_fault.fault.handler_pc+1, sched_fault.tid);


		strcpy(buffer, "ISR Caller");
		posix_trace_event_addr_tid(
				POSIX_TRACE_MESSAGE,
				buffer, strlen(buffer),
				(uint32_t)sched_fault.fault.handler_caller,
				sched_fault.tid);
		mcu_debug("ISR Caller 0x%lX %ld\n", (u32)sched_fault.fault.handler_caller, sched_fault.tid);

		mcu_core_privcall(priv_fault_logged, NULL);

	}

	return 0;
}

void priv_check_sleep_mode(void * args){
	bool * p = (bool*)args;
	int i;
	*p = true;
	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) && sched_active_asserted(i) ){
			*p = false;
			return;
		}
	}
}

void sched_priv_update_on_stopped(){
	int i;
	int new_priority;

	sched_current_priority = SCHED_LOWEST_PRIORITY - 1;
	new_priority = sched_current_priority;
	//start with process 1 -- 0 is the scheduler process
	for(i=1; i < task_get_total(); i++){
		//Find the highest priority of all active processes
		if ( task_enabled(i) &&
				sched_active_asserted(i) &&
				!sched_stopped_asserted(i) &&
				(sched_get_priority(i) > new_priority) ){
			new_priority = sched_get_priority(i);
		}
	}

	if ( new_priority >= SCHED_LOWEST_PRIORITY ){
		sched_priv_update_on_wake(new_priority);
	} else {
		task_priv_switch_context(NULL);
	}
}


void sched_priv_update_on_sleep(){
	sched_priv_deassert_active( task_get_current() );
	sched_priv_update_on_stopped();
}

void sched_priv_update_on_wake(int new_priority){
	int i;
	bool switch_context;

	if (new_priority > sched_current_priority){
		switch_context = true;
		sched_current_priority = new_priority;
	} else if (new_priority == sched_current_priority){
		switch_context = false;
	} else {
		sched_priv_assert_status_change();
		return;
	}

	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) && sched_active_asserted(i) && !sched_stopped_asserted(i) && ( sched_get_priority(i) == sched_current_priority ) ){
			//Enable process execution for highest active priority tasks
			task_assert_exec(i);
		} else {
			//Disable process execution for lower priority tasks
			task_deassert_exec(i);
		}
	}

	if ( switch_context == true ){
		task_priv_switch_context(NULL);
	}
}


int sched_get_highest_priority_blocked(void * block_object){
	int priority;
	int i;
	int new_thread;

	//check to see if another task is waiting for the mutex
	priority = SCHED_LOWEST_PRIORITY - 1;
	new_thread = -1;
	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) ){
			if ( (stratify_sched_table[i].block_object == block_object) && ( !sched_active_asserted(i) ) ){
				//it's waiting for the block -- give the block to the highest priority and waiting longest
				if ( stratify_sched_table[i].attr.schedparam.sched_priority > priority ){

					//! \todo Find the task that has been waiting the longest time
					new_thread = i;
					priority = stratify_sched_table[i].attr.schedparam.sched_priority;
				}
			}
		}
	}
	return new_thread;
}

int sched_priv_unblock_all(void * block_object, int unblock_type){
	int i;
	int priority;
	priority = SCHED_LOWEST_PRIORITY - 1;
	for(i=1; i < task_get_total(); i++){
		if ( task_enabled(i) ){
			if ( (stratify_sched_table[i].block_object == block_object) && ( !sched_active_asserted(i) ) ){
				//it's waiting for the semaphore -- give the semaphore to the highest priority and waiting longest
				sched_priv_assert_active(i, unblock_type);
				if ( stratify_sched_table[i].attr.schedparam.sched_priority > priority ){
					priority = stratify_sched_table[i].attr.schedparam.sched_priority;
				}
			}
		}
	}
	return priority;
}

int start_first_thread(){
	void * (*init)(void*);
	pthread_attr_t attr;
	int err;

	init = stratify_sched_table[0].init;

	attr.stacksize = stratify_board_config.start_stack_size;
	attr.stackaddr = malloc(attr.stacksize);
	if ( attr.stackaddr == NULL ){
		errno = ENOMEM;
		return -1;
	}
	PTHREAD_ATTR_SET_IS_INITIALIZED((&attr), 1);
	PTHREAD_ATTR_SET_CONTENTION_SCOPE((&attr), PTHREAD_SCOPE_SYSTEM);
	PTHREAD_ATTR_SET_GUARDSIZE((&attr), SCHED_DEFAULT_STACKGUARD_SIZE);
	PTHREAD_ATTR_SET_INHERIT_SCHED((&attr), PTHREAD_EXPLICIT_SCHED);
	PTHREAD_ATTR_SET_DETACH_STATE((&attr), PTHREAD_CREATE_DETACHED);
	PTHREAD_ATTR_SET_SCHED_POLICY((&attr), SCHED_RR);
	attr.schedparam.sched_priority = 21; //not the default priority

	err = sched_new_thread(init,
			stratify_board_config.start_args,
			attr.stackaddr,
			attr.stacksize,
			&attr);

	if ( !err ){
		return -1;
	}

	return 0;
}




/*! @} */

