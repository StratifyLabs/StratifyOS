// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef SCHED_FLAGS_H_
#define SCHED_FLAGS_H_

#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>

#include "cortexm/cortexm.h"
#include "cortexm/fault.h"
#include "cortexm/task.h"

#include "sos/debug.h"
#include "sos/sos.h"
#include "trace.h"

#include "scheduler_flags.h"
#include "scheduler_timing.h"
#include "scheduler_fault.h"
#include "scheduler.h"


#define SCHEDULER_NUM_SIGNALS 32


static inline int scheduler_priority(int id){ return task_get_priority(id); }
static inline trace_id_t scheduler_trace_id(int id) {
  return sos_sched_table[id].trace_id;
}
static inline int scheduler_current_priority(){ return task_get_current_priority(); }

int scheduler_check_tid(int id);
void scheduler_prepare();
void scheduler();

int scheduler_create_thread(void *(*p)(void*)  /*! The function to execute for the task */,
		void * arg /*! The thread's single argument */,
		void * mem_addr /*! The address for the new thread memory */,
		int mem_size /*! The heap/stack size in bytes */,
		const pthread_attr_t * attr);

void scheduler_thread_cleanup(void * status);

int scheduler_create_process(void (*p)(char *),
		const char * path_arg,
		task_memories_t * mem,
		void * reent, int parent_id, int is_root);

int scheduler_switch_context(void * args);
int scheduler_get_highest_priority_blocked(void * block_object);


void scheduler_check_cancellation();

#endif /* SCHED_FLAGS_H_ */
