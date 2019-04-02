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


#ifndef SCHED_FLAGS_H_
#define SCHED_FLAGS_H_

#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include "cortexm/fault.h"
#include "cortexm/task.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "sos/sos.h"
#include "trace.h"

#include "scheduler_flags.h"
#include "scheduler_timing.h"
#include "scheduler_fault.h"
#include "scheduler.h"


#define SCHEDULER_NUM_SIGNALS 32

extern volatile scheduler_fault_t m_scheduler_fault;


static inline int scheduler_priority(int id){ return task_get_priority(id); }
static inline trace_id_t scheduler_trace_id(int id){ return sos_sched_table[id].trace_id; }
static inline int scheduler_current_priority(){ return task_get_current_priority(); }

int scheduler_check_tid(int id);
int scheduler_prepare();
void scheduler();

int scheduler_create_thread(void *(*p)(void*)  /*! The function to execute for the task */,
		void * arg /*! The thread's single argument */,
		void * mem_addr /*! The address for the new thread memory */,
		int mem_size /*! The heap/stack size in bytes */,
		const pthread_attr_t * attr);


int scheduler_create_process(void (*p)(char *),
		const char * path_arg,
		task_memories_t * mem,
		void * reent, int parent_id);

int scheduler_switch_context(void * args);
int scheduler_get_highest_priority_blocked(void * block_object);

#endif /* SCHED_FLAGS_H_ */
