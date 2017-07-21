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
#include <pthread.h>
#include <reent.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>

#include "mcu/debug.h"

#include "sched_flags.h"
#include "../syscalls/malloc_local.h"


static void cleanup_thread(void * status);

typedef struct {
	int joined;
	int status;
} priv_wait_joined_t;
static void priv_wait_joined(void * args) MCU_PRIV_EXEC_CODE;
static void priv_cleanup(void * args) MCU_PRIV_EXEC_CODE;

typedef struct {
	int id;
	pthread_attr_t * attr;
	void * stackguard;
} priv_activate_thread_t;
static void priv_activate_thread(priv_activate_thread_t * args) MCU_PRIV_EXEC_CODE;


/*! \details This function creates a new thread.
 * \return The thread id or zero if the thread could not be created.
 */
int sched_new_thread(void *(*p)(void*)  /*! The function to execute for the task */,
		void * arg /*! The thread's single argument */,
		void * mem_addr /*! The address for the thread memory (bottom of the stack) */,
		int mem_size /*! The stack size in bytes */,
		pthread_attr_t * attr){
	int id;
	struct _reent * reent;
	priv_activate_thread_t args;

	//start a new thread
	id = task_new_thread(p, cleanup_thread, arg, mem_addr, mem_size, task_get_pid( task_get_current() ) );

	if ( id > 0 ){
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
		mcu_core_privcall((core_privcall_t)priv_activate_thread, &args);
	}
	return id;
}


void priv_activate_thread(priv_activate_thread_t * args){
	int id;
	id = args->id;
	struct _reent * reent;

	memset( (void*)&sos_sched_table[id], 0, sizeof(sched_task_t));
	memcpy( (void*)&(sos_sched_table[id].attr), args->attr, sizeof(pthread_attr_t));

	//Items inherited from parent thread
	//Signal mask

	reent = (struct _reent *)task_table[id].reent;
	reent->sigmask = _REENT->sigmask;

	sos_sched_table[args->id].priority = args->attr->schedparam.sched_priority;
	if ( PTHREAD_ATTR_GET_SCHED_POLICY( (&(sos_sched_table[id].attr)) ) == SCHED_FIFO ){
		task_assert_isfifo(id);
	} else {
		task_deassert_isfifo(id);
	}

	sos_sched_table[id].wake.tv_sec = SCHED_TIMEVAL_SEC_INVALID;
	sos_sched_table[id].wake.tv_usec = 0;
	sched_priv_assert_active(id, 0);
	sched_priv_assert_inuse(id);
#if USE_MEMORY_PROTECTION > 0
	task_priv_set_stackguard(id, args->stackguard, SCHED_DEFAULT_STACKGUARD_SIZE);
#endif
	sched_priv_update_on_wake(sos_sched_table[id].priority);
}

void cleanup_thread(void * status){
	int detach_state;
	priv_wait_joined_t args;

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
		do {
			mcu_core_privcall(priv_wait_joined, &args);
		} while(args.joined == 0);
	}

	//Free all memory associated with this thread
	malloc_free_task_r(_REENT, task_get_current() );
	free( sos_sched_table[task_get_current()].attr.stackaddr ); //free the stack address
	mcu_core_privcall(priv_cleanup, &args.joined);
}

void priv_wait_joined(void * args){
	int joined;
	priv_wait_joined_t * p = (priv_wait_joined_t*)args;

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
		sched_priv_update_on_sleep();
	} else {
		sos_sched_table[task_get_current()].exit_status = p->status;
	}
}

void priv_cleanup(void * args){
	int joined;
	//notify all joined threads of termination
	for(joined=1; joined < task_get_total(); joined++){
		//check to see if any threads are blocked on this thread
		if ( sos_sched_table[joined].block_object == &sos_sched_table[task_get_current()] ){
			//This thread is joined to the current thread
			sos_sched_table[joined].exit_status = sos_sched_table[task_get_current()].exit_status;
			sched_priv_assert_active(joined, SCHED_UNBLOCK_PTHREAD_JOINED_THREAD_COMPLETE);
		}
	}

	sos_sched_table[task_get_current()].flags = 0;
	task_priv_del(task_get_current());
	sched_priv_update_on_sleep();
}

/*! @} */
