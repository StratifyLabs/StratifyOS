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


#include "config.h"
#include <pthread.h>
#include <reent.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/malloc/malloc_local.h>

#include "mcu/debug.h"
#include "scheduler_local.h"

static void cleanup_thread(void * status);

typedef struct {
	int joined;
	int status;
} svcall_wait_joined_t;
static void svcall_wait_joined(void * args) MCU_ROOT_EXEC_CODE;
static void svcall_cleanup(void * args) MCU_ROOT_EXEC_CODE;
void svcall_elevate_priority(void * args) MCU_ROOT_EXEC_CODE;

typedef struct {
	int id;
	const pthread_attr_t * attr;
	void * stackguard;
} svcall_activate_thread_t;
static void svcall_activate_thread(svcall_activate_thread_t * args) MCU_ROOT_EXEC_CODE;
static void activate_thread(int id, void * mem_addr, const pthread_attr_t * attr);

int scheduler_create_thread(
		void *(*p)(void*),
		void * arg,
		void * mem_addr,
		int mem_size,
		const pthread_attr_t * attr
		){
	int id;

	//start a new thread
	id = task_create_thread(
				p,
				cleanup_thread,
				arg,
				mem_addr,
				mem_size,
				task_get_pid( task_get_current() )
				);


	if ( id > 0 ){
		activate_thread(id, mem_addr, attr);
	}

	return id;
}

void activate_thread(int id, void * mem_addr, const pthread_attr_t * attr){
	struct _reent * reent;
	svcall_activate_thread_t args;

	//Initialize the reent structure
	reent = (struct _reent *)mem_addr; //The bottom of the stack
	_REENT_INIT_PTR(reent);

	reent->_stderr = _GLOBAL_REENT->_stderr;
	reent->_stdout = _GLOBAL_REENT->_stdout;
	reent->_stdin = _GLOBAL_REENT->_stdin;
	reent->__sdidinit = 1;

	reent->procmem_base = _GLOBAL_REENT->procmem_base; //malloc use the same base as the process

	//Now activate the thread
	args.id = id;
	args.attr = attr;
	args.stackguard = (void*)((uint32_t)mem_addr + sizeof(struct _reent));

	cortexm_svcall((cortexm_svcall_t)svcall_activate_thread, &args);

}


void svcall_activate_thread(svcall_activate_thread_t * args){
	CORTEXM_SVCALL_ENTER();
	int id;
	id = args->id;
	struct _reent * reent;


	memset( (void*)&sos_sched_table[id], 0, sizeof(sched_task_t));
	memcpy( (void*)&(sos_sched_table[id].attr), args->attr, sizeof(pthread_attr_t));

	//Items inherited from parent thread
	//Signal mask
	reent = (struct _reent *)sos_task_table[id].reent;
	reent->sigmask = _REENT->sigmask;

	task_set_priority( args->id, args->attr->schedparam.sched_priority );
	if ( PTHREAD_ATTR_GET_SCHED_POLICY( (&(sos_sched_table[id].attr)) ) == SCHED_FIFO ){
		task_assert_fifo(id);
	} else {
		task_deassert_fifo(id);
	}

	scheduler_timing_root_process_timer_initialize(id);
	sos_sched_table[id].wake.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
	sos_sched_table[id].wake.tv_usec = 0;
	scheduler_root_assert_active(id, 0);
	scheduler_root_assert_inuse(id);
	if( scheduler_authenticated_asserted(task_get_current()) ){
		scheduler_root_assert_authenticated(id);
	}

	if( (sos_board_config.o_sys_flags & SYS_FLAG_IS_FIRST_THREAD_ROOT) &&
		 (id == 1) ){
		scheduler_root_assert_authenticated(1);
	}


	if( task_root_set_stackguard(id, args->stackguard, SCHED_DEFAULT_STACKGUARD_SIZE) < 0 ){
		mcu_debug_log_warning(MCU_DEBUG_SCHEDULER, "Failed to activate stack guard");
	}
	scheduler_root_update_on_wake(id, task_get_priority(id));
}

void cleanup_thread(void * status){
	int detach_state;
	svcall_wait_joined_t args;

	//null these so they aren't cleaned up because they are shared with other threads in the process
	stdin = 0;
	stdout = 0;
	stderr = 0;

	//This will close any other open files
	if ( _REENT->__cleanup ){
		_REENT->__cleanup(_REENT);
	}

	detach_state = PTHREAD_ATTR_GET_DETACH_STATE( (&(sos_sched_table[task_get_current()].attr)) );
	args.joined = 0;
	if ( detach_state == PTHREAD_CREATE_JOINABLE ){
		args.status = (int)status;
		mcu_debug_log_info(MCU_DEBUG_SCHEDULER, "Wait joined");
		do {
			cortexm_svcall(svcall_wait_joined, &args);
		} while(args.joined == 0);
	}

	mcu_debug_log_info(MCU_DEBUG_SCHEDULER, "%d:Shutdown", task_get_current());
	//Free all memory associated with this thread
	malloc_free_task_r(_REENT, task_get_current() );

	/*
	  * Once the stack has been freed (which can't be done in root mode), the root cleanup
	  * must complete before a context switch is allowed. To prevent a context switch,
	  * the thread will be switch to SCHED_FIFO with the max priority
	  *
	  */

	cortexm_svcall(svcall_elevate_priority, 0);
	//The stack is allocated by a different thread so it isn't freed with malloc_free_task_r()
	_free_r( _REENT, sos_sched_table[task_get_current()].attr.stackaddr ); //free the stack address
	cortexm_svcall(svcall_cleanup, &args.joined);
}

void svcall_elevate_priority(void * args){
	CORTEXM_SVCALL_ENTER();
	u32 id = task_get_current();
	//Issue #161 -- need to set the effective priority -- not just the prio ceiling
	PTHREAD_ATTR_SET_SCHED_POLICY( (&(sos_sched_table[id].attr)), SCHED_FIFO);
	sos_sched_table[id].attr.schedparam.sched_priority = SCHED_HIGHEST_PRIORITY;
	task_set_priority(id, SCHED_HIGHEST_PRIORITY);
	task_assert_fifo(id);
	scheduler_root_update_on_stopped(); //this will cause the context switcher to update the active priority
}

void svcall_wait_joined(void * args){
	CORTEXM_SVCALL_ENTER();
	int joined;
	svcall_wait_joined_t * p = (svcall_wait_joined_t*)args;

	//wait until the thread has been joined to free the resources
	for(joined=1; joined < task_get_total(); joined++){
		//check to see if any threads are blocked on this thread
		if ( sos_sched_table[joined].block_object == &sos_sched_table[task_get_current()] ){
			//This thread is joined to the current thread
			p->joined = joined;
			//the thread can continue when one thread has been joined
			break;
		}
	}

	if ( p->joined == 0 ){
		sos_sched_table[task_get_current()].block_object = (void*)&sos_sched_table[task_get_current()].block_object; //block on self
		scheduler_root_update_on_sleep();
	} else {
		sos_sched_table[task_get_current()].exit_status = p->status;
	}
}

void svcall_cleanup(void * args){
	CORTEXM_SVCALL_ENTER();
	int joined;

	//notify all joined threads of termination
	for(joined=1; joined < task_get_total(); joined++){
		//check to see if any threads are blocked on this thread
		if ( sos_sched_table[joined].block_object == &sos_sched_table[task_get_current()] ){
			//This thread is joined to the current thread
			sos_sched_table[joined].exit_status = sos_sched_table[task_get_current()].exit_status;
			scheduler_root_assert_active(joined, SCHEDULER_UNBLOCK_PTHREAD_JOINED_THREAD_COMPLETE);
		}
	}

	sos_sched_table[task_get_current()].flags = 0;
	task_root_delete(task_get_current());
	scheduler_root_update_on_sleep();
}

