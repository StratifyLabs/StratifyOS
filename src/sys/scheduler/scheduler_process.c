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

#include "config.h"
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>

#include "scheduler_local.h"

typedef struct {
	task_memories_t * mem;
	int tid;
} init_sched_task_t;

static void root_init_sched_task(init_sched_task_t * task) MCU_ROOT_EXEC_CODE;
static void cleanup_process(void * status);

/*! \details This function creates a new process.
 * \return The thread id or zero if the thread could not be created.
 */
int scheduler_create_process(void (*p)(char *)  /*! The startup function (crt()) */,
									  const char * path_arg /*! Path string with arguments */,
									  task_memories_t * mem,
									  void * reent /*! The location of the reent structure */,
									  int parent_id){
	int tid;
	init_sched_task_t args;

	//Start a new process
	tid = task_create_process(
				p,
				cleanup_process,
				path_arg,
				mem,
				reent,
				parent_id);

	if ( tid > 0 ){
		//update the scheduler table using a privileged call
		args.tid = tid;
		args.mem = mem;
		cortexm_svcall((cortexm_svcall_t)root_init_sched_task, &args);
	} else {
		return -1;
	}

	mcu_debug_log_info(MCU_DEBUG_SYS, "start process id:%d pid:%d parent:%d (%d)", tid, task_get_pid(tid), task_get_parent(tid), task_get_current());
	return task_get_pid( tid );
}

void root_init_sched_task(init_sched_task_t * task){
	uint32_t stackguard;
	struct _reent * reent;
	int id = task->tid;
	memset((void*)&sos_sched_table[id], 0, sizeof(sched_task_t));

	PTHREAD_ATTR_SET_IS_INITIALIZED((&(sos_sched_table[id].attr)), 1);
	PTHREAD_ATTR_SET_SCHED_POLICY((&(sos_sched_table[id].attr)), SCHED_OTHER);
	PTHREAD_ATTR_SET_GUARDSIZE((&(sos_sched_table[id].attr)), SCHED_DEFAULT_STACKGUARD_SIZE);
	PTHREAD_ATTR_SET_CONTENTION_SCOPE((&(sos_sched_table[id].attr)), PTHREAD_SCOPE_SYSTEM);
	PTHREAD_ATTR_SET_INHERIT_SCHED((&(sos_sched_table[id].attr)), PTHREAD_EXPLICIT_SCHED);
	PTHREAD_ATTR_SET_DETACH_STATE((&(sos_sched_table[id].attr)), PTHREAD_CREATE_DETACHED);

	sos_sched_table[id].attr.stackaddr = task->mem->data.addr; //Beginning of process data memory
	sos_sched_table[id].attr.stacksize = task->mem->data.size; //Size of the memory (not just the stack)
	sos_sched_table[id].attr.schedparam.sched_priority = 0; //This is the priority to revert to after being escalated

	scheduler_timing_root_process_timer_initialize(id);

	sos_sched_table[id].wake.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
	sos_sched_table[id].wake.tv_usec = 0;
	scheduler_root_assert_active(id, 0);
	scheduler_root_assert_inuse(id);
	scheduler_root_update_on_wake(id, task_get_priority(id));
	stackguard = (uint32_t)task->mem->data.addr + sizeof(struct _reent);
	if( task_root_set_stackguard(id, (void*)stackguard, SCHED_DEFAULT_STACKGUARD_SIZE) < 0 ){
		mcu_debug_log_warning(MCU_DEBUG_SCHEDULER, "Failed to set stackguard");
	}

	//Items inherited from parent process

	//Signal mask
	reent = (struct _reent *)sos_task_table[id].reent;
	reent->sigmask = _REENT->sigmask;
}

static void cleanup_process(void * status){
	//Processes should ALWAYS use exit -- this should never get called but is here just in case
	kill(task_get_pid(task_get_current()), SIGKILL);
}

