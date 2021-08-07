// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef SCHEDULER_SCHEDULER_LOCAL_H_
#define SCHEDULER_SCHEDULER_LOCAL_H_

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

#include "scheduler.h"

#include "sos_config.h"

#define SCHEDULER_TASK_FLAG_UNBLOCK_MASK 0x0F // bits 0 to 3 are unblock type
// 4 is available
#define SCHEDULER_TASK_FLAG_INUSE 5
#define SCHEDULER_TASK_FLAG_WAITCHILD 6
#define SCHEDULER_TASK_FLAG_SIGCAUGHT 7
#define SCHEDULER_TASK_FLAG_AIOSUSPEND 8
#define SCHEDULER_TASK_FLAG_ZOMBIE 9
#define SCHEDULER_TASK_FLAG_LISTIOSUSPEND 10
#define SCHEDULER_TASK_FLAG_CANCEL_ASYNCHRONOUS 11
#define SCHEDULER_TASK_FLAG_CANCEL_ENABLE 12
#define SCHEDULER_TASK_FLAG_AUTHENTICATED 13
#define SCHEDULER_TASK_FLAG_CANCEL 14

#define SCHEDULER_TIMEVAL_SEC_INVALID UINT_MAX
#define SCHEDULER_TIMEVAL_SECONDS SOS_SCHEDULER_TIMEVAL_SECONDS

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

// not used for porting, just needs to be here
typedef struct {
  u32 o_flags;
  struct mcu_timeval value;
  struct mcu_timeval interval;
  struct sigevent sigevent;
} sos_process_timer_t;

// not used for porting, just needs to be here
typedef struct {
  pthread_attr_t attr;
  volatile void *block_object;
  union {
    volatile int exit_status;
    void *(*init)(void *);
  };
  pthread_mutex_t *signal_delay_mutex;
  volatile struct mcu_timeval wake;
  volatile u16 flags;
  trace_id_t trace_id;
  sos_process_timer_t timer[CONFIG_TASK_PROCESS_TIMER_COUNT];
} sched_task_t;

extern volatile sched_task_t sos_sched_table[];



static inline int scheduler_priority(int id){ return task_get_priority(id); }
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
                void * reent, int parent_id);

int scheduler_switch_context(void * args);
int scheduler_get_highest_priority_blocked(void * block_object);


void scheduler_check_cancellation();

extern void show_mpu(void *);


static inline trace_id_t scheduler_trace_id(int id) {
  return sos_sched_table[id].trace_id;
}


static inline int scheduler_cancel_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_CANCEL); }
static inline int scheduler_cancel_enable_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_CANCEL_ENABLE); }
static inline int scheduler_cancel_asynchronous_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_CANCEL_ASYNCHRONOUS); }
static inline int scheduler_listiosuspend_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_LISTIOSUSPEND); }
static inline int scheduler_waitchild_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_WAITCHILD); }

static inline int scheduler_inuse_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_INUSE); }
static inline int scheduler_sigcaught_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_SIGCAUGHT); }
static inline int scheduler_aiosuspend_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_AIOSUSPEND); }
static inline int scheduler_zombie_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_ZOMBIE); }
static inline int scheduler_authenticated_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_AUTHENTICATED); }

static inline volatile int scheduler_unblock_type(int id) MCU_ALWAYS_INLINE;
volatile int scheduler_unblock_type(int id) {
  return sos_sched_table[id].flags & SCHEDULER_TASK_FLAG_UNBLOCK_MASK;
}


#endif /* SCHEDULER_SCHEDULER_LOCAL_H_ */
