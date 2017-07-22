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


#ifndef SCHED_FLAGS_H_
#define SCHED_FLAGS_H_

#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include "mcu/fault.h"
#include "mcu/task.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "sos/sos.h"
#include "trace.h"
#include "sched.h"

#define SCHED_TASK_FLAGS_ACTIVE 4
#define SCHED_TASK_FLAGS_INUSE 5
#define SCHED_TASK_FLAGS_WAITCHILD 6
#define SCHED_TASK_FLAGS_SIGCAUGHT 7
#define SCHED_TASK_FLAGS_AIOSUSPEND 8
#define SCHED_TASK_FLAGS_ZOMBIE 9
#define SCHED_TASK_FLAGS_LISTIOSUSPEND 10
#define SCHED_TASK_FLAGS_STOPPED 11
#define SCHED_TASK_FLAGS_PRIV_SYNC 12
#define SCHED_TASK_FLAGS_UNBLOCK_MASK 0x0F

typedef enum {
	SCHED_UNBLOCK_NONE,
	SCHED_UNBLOCK_MUTEX,
	SCHED_UNBLOCK_SEMAPHORE,
	SCHED_UNBLOCK_RWLOCK,
	SCHED_UNBLOCK_COND,
	SCHED_UNBLOCK_SLEEP,
	SCHED_UNBLOCK_WAIT,
	SCHED_UNBLOCK_SIGNAL,
	SCHED_UNBLOCK_TRANSFER,
	SCHED_UNBLOCK_MQ,
	SCHED_UNBLOCK_PTHREAD_JOINED,
	SCHED_UNBLOCK_PTHREAD_JOINED_THREAD_COMPLETE,
	SCHED_UNBLOCK_AIO
} sched_unblock_type_t;

#define SCHED_TIMEVAL_SEC_INVALID UINT_MAX
#define SCHED_TIMEVAL_SECONDS STFY_SCHED_TIMEVAL_SECONDS

#define SCHED_NUM_SIGNALS 32

//Scheduler timing
int sched_timing_init();

extern volatile i8 sched_current_priority;
extern volatile i8 sched_status_changed;
extern volatile sched_fault_t sched_fault;

static inline int sched_active_asserted(int id) MCU_ALWAYS_INLINE;
int sched_active_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_ACTIVE); };

static inline int sched_sync_asserted(int id) MCU_ALWAYS_INLINE;
int sched_sync_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_PRIV_SYNC); };


static inline int sched_inuse_asserted(int id) MCU_ALWAYS_INLINE;
int sched_inuse_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_INUSE); };

static inline int sched_waitchild_asserted(int id) MCU_ALWAYS_INLINE;
int sched_waitchild_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_WAITCHILD); };

static inline int sched_sigcaught_asserted(int id) MCU_ALWAYS_INLINE;
int sched_sigcaught_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_SIGCAUGHT); };

static inline int sched_aiosuspend_asserted(int id) MCU_ALWAYS_INLINE;
int sched_aiosuspend_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_AIOSUSPEND); };

static inline int sched_listiosuspend_asserted(int id) MCU_ALWAYS_INLINE;
int sched_listiosuspend_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_LISTIOSUSPEND); };

static inline int sched_stopped_asserted(int id) MCU_ALWAYS_INLINE;
int sched_stopped_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_STOPPED); };

static inline int sched_zombie_asserted(int id) MCU_ALWAYS_INLINE;
int sched_zombie_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHED_TASK_FLAGS_ZOMBIE); };

static inline int sched_get_priority(int id) MCU_ALWAYS_INLINE;
int sched_get_priority(int id){ return sos_sched_table[id].priority; };

static inline trace_id_t sched_get_trace_id(int id) MCU_ALWAYS_INLINE;
trace_id_t sched_get_trace_id(int id){ return sos_sched_table[id].trace_id; };


static inline int sched_get_current_priority() MCU_ALWAYS_INLINE;
int sched_get_current_priority(){ return sched_current_priority; };

static inline int sched_status_change_asserted() MCU_ALWAYS_INLINE;
inline int sched_status_change_asserted(){
	return sched_status_changed;
}

static inline volatile int sched_get_unblock_type(int id) MCU_ALWAYS_INLINE;
volatile int sched_get_unblock_type(int id){ return sos_sched_table[id].flags & SCHED_TASK_FLAGS_UNBLOCK_MASK; };

static inline void sched_priv_set_unblock_type(int id, sched_unblock_type_t unblock_type) MCU_ALWAYS_INLINE;
void sched_priv_set_unblock_type(int id, sched_unblock_type_t unblock_type){
	sos_sched_table[id].flags &= ~SCHED_TASK_FLAGS_UNBLOCK_MASK;
	sos_sched_table[id].flags |= unblock_type;
};

void sched_priv_update_on_sleep();
void sched_priv_update_on_stopped();
void sched_priv_update_on_wake(int new_priority);
void sched_priv_cleanup_tasks(void * args);

void sched_priv_assert(int id, int flag);
void sched_priv_deassert(int id, int flag);

void sched_priv_assert_active(int id, int unblock_type);
void sched_priv_deassert_active(int id);
void sched_priv_assert_status_change();
void sched_priv_deassert_status_change();
void sched_priv_set_current_priority(int priority);
void sched_priv_set_trace_id(int tid, trace_id_t id);


static inline void sched_priv_assert_waitchild(int id);
void sched_priv_assert_waitchild(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_WAITCHILD);
}
static inline void sched_priv_deassert_waitchild(int id);
void sched_priv_deassert_waitchild(int id){
	sched_priv_deassert(id, SCHED_TASK_FLAGS_WAITCHILD);
}

static inline void sched_priv_assert_inuse(int id);
void sched_priv_assert_inuse(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_INUSE);
}
static inline void sched_priv_deassert_inuse(int id);
void sched_priv_deassert_inuse(int id){
	sched_priv_deassert(id, SCHED_TASK_FLAGS_INUSE);
}
static inline void sched_priv_assert_sigcaught(int id);
void sched_priv_assert_sigcaught(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_SIGCAUGHT);
}
static inline void sched_priv_deassert_sigcaught(int id);
void sched_priv_deassert_sigcaught(int id){
	sched_priv_deassert(id, SCHED_TASK_FLAGS_SIGCAUGHT);
}
static inline void sched_priv_assert_aiosuspend(int id);
void sched_priv_assert_aiosuspend(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_AIOSUSPEND);
}
static inline void sched_priv_deassert_aiosuspend(int id);
void sched_priv_deassert_aiosuspend(int id){
	sched_priv_deassert(id, SCHED_TASK_FLAGS_AIOSUSPEND);
}
static inline void sched_priv_assert_listiosuspend(int id);
void sched_priv_assert_listiosuspend(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_LISTIOSUSPEND);
}
static inline void sched_priv_deassert_listiosuspend(int id);
void sched_priv_deassert_listiosuspend(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_LISTIOSUSPEND);
}

static inline  void sched_priv_assert_stopped(int id);
void sched_priv_assert_stopped(int id){
	sched_priv_assert(id, SCHED_TASK_FLAGS_STOPPED);
}
static inline void sched_priv_deassert_stopped(int id);
void sched_priv_deassert_stopped(int id){
	sched_priv_deassert(id, SCHED_TASK_FLAGS_STOPPED);
}

static inline void sched_priv_deassert_sync(int id);
void sched_priv_deassert_sync(int id){
	sched_priv_deassert(id, SCHED_TASK_FLAGS_PRIV_SYNC);
}

int sched_check_tid(int id);
void sched_microsecond_match_event(int current_match);
void sched_second_match_event();

int sched_send_signal(int id, int sig);
int sched_send_task_signal(int tid, int sig);

/*! \details This function uses the task API to create a new thread.
 * \return Zero on success
 */
int sched_new_thread(void *(*p)(void*)  /*! The function to execute for the task */,
		void * arg /*! The thread's single argument */,
		void * mem_addr /*! The address for the new thread memory */,
		int mem_size /*! The stack size in bytes */,
		pthread_attr_t * attr);


int sched_new_process(void (*p)(char *),
		const char * path_arg,
		task_memories_t * mem,
		void * reent);

int sched_update();

void sched_priv_abnormal_child_stop(int id);
void sched_priv_stop_task(int id);
void sched_priv_start_task(int id);
void sched_notify_parent_of_kill(int id);
int sched_set_param(int id, int priority, int policy);
int sched_get_param(int id, int * priority, int * policy);
int sched_switch_context(void * args);

void sched_priv_assert_sync(void * args) MCU_PRIV_CODE;

int sched_get_highest_priority_blocked(void * block_object);
int sched_priv_unblock_all(void * block_object, int unblock_type);

void sched_priv_timedblock(void * block_object, struct sched_timeval * interval);

uint32_t sched_seconds_to_clocks(int seconds);
uint32_t sched_useconds_to_clocks(int useconds);
uint32_t sched_nanoseconds_to_clocks(int nanoseconds);
void sched_convert_timespec(struct sched_timeval * tv, const struct timespec * ts);
void sched_priv_get_realtime(struct sched_timeval * tv) MCU_PRIV_EXEC_CODE;
void sched_fault_build_string(char * dest);
void sched_fault_build_trace_string(char * dest);

int sched_prepare();
void scheduler();

#define SCHED_DEBUG 0
#define sched_debug(...) do { if ( SCHED_DEBUG == 1 ){ mcu_debug("%s:", __func__); mcu_debug(__VA_ARGS__); } } while(0)
#define sched_priv_debug(...) do { if ( SCHED_DEBUG == 1 ){ mcu_priv_debug("%s:", __func__); mcu_priv_debug(__VA_ARGS__); } } while(0)

void sched_priv_set_delaymutex(void * args) MCU_PRIV_EXEC_CODE;

#define SCHED_CLK_NSEC_DIV ((u32)((u64)1024 * 1000000000 / mcu_board_config.core_cpu_freq))
#define SCHED_CLK_USEC_MULT ((u32)(mcu_board_config.core_cpu_freq / 1000000))



#endif /* SCHED_FLAGS_H_ */
