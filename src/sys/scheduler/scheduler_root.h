/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef SCHEDULER_SCHEDULER_ROOT_H_
#define SCHEDULER_SCHEDULER_ROOT_H_

#include "mcu/types.h"

#define SCHEDULER_TASK_FLAG_INUSE 5
#define SCHEDULER_TASK_FLAG_WAITCHILD 6
#define SCHEDULER_TASK_FLAG_SIGCAUGHT 7
#define SCHEDULER_TASK_FLAG_AIOSUSPEND 8
#define SCHEDULER_TASK_FLAG_ZOMBIE 9
#define SCHEDULER_TASK_FLAG_LISTIOSUSPEND 10
#define SCHEDULER_TASK_FLAG_ROOT_SYNC 12
#define SCHEDULER_TASK_FLAG_UNBLOCK_MASK 0x0F

typedef enum {
	SCHEDULER_UNBLOCK_NONE,
	SCHEDULER_UNBLOCK_MUTEX,
	SCHEDULER_UNBLOCK_SEMAPHORE,
	SCHEDULER_UNBLOCK_RWLOCK,
	SCHEDULER_UNBLOCK_COND,
	SCHEDULER_UNBLOCK_SLEEP,
	SCHEDULER_UNBLOCK_WAIT,
	SCHEDULER_UNBLOCK_SIGNAL,
	SCHEDULER_UNBLOCK_TRANSFER,
	SCHEDULER_UNBLOCK_MQ,
	SCHEDULER_UNBLOCK_PTHREAD_JOINED,
	SCHEDULER_UNBLOCK_PTHREAD_JOINED_THREAD_COMPLETE,
	SCHEDULER_UNBLOCK_AIO
} scheduler_unblock_type_t;

void scheduler_root_assert(int id, int flag);
void scheduler_root_deassert(int id, int flag);

void scheduler_root_update_on_sleep();
void scheduler_root_update_on_stopped();
void scheduler_root_update_on_wake(int id, int new_priority);

void scheduler_root_assert_active(int id, int unblock_type);
void scheduler_root_deassert_active(int id);
void scheduler_root_set_trace_id(int tid, trace_id_t id);
void scheduler_root_assert_sync(void * args) MCU_ROOT_CODE;
int scheduler_root_unblock_all(void * block_object, int unblock_type);
void scheduler_root_set_delaymutex(void * args) MCU_ROOT_EXEC_CODE;

void scheduler_root_stop_task(int id);
void scheulder_root_start_task(int id);

static inline volatile int scheduler_unblock_type(int id) MCU_ALWAYS_INLINE;
volatile int scheduler_unblock_type(int id){ return sos_sched_table[id].flags & SCHEDULER_TASK_FLAG_UNBLOCK_MASK; }


static inline void scheduler_root_set_unblock_type(int id, scheduler_unblock_type_t unblock_type) MCU_ALWAYS_INLINE;
void scheduler_root_set_unblock_type(int id, scheduler_unblock_type_t unblock_type){
	sos_sched_table[id].flags &= ~SCHEDULER_TASK_FLAG_UNBLOCK_MASK;
	sos_sched_table[id].flags |= unblock_type;
};



#endif /* SCHEDULER_SCHEDULER_ROOT_H_ */
