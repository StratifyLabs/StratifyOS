// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SCHEDULER_SCHEDULER_ROOT_H_
#define SCHEDULER_SCHEDULER_ROOT_H_

#include "scheduler_local.h"

void scheduler_root_assert(int id, int flag);
void scheduler_root_deassert(int id, int flag);

void scheduler_root_update_on_sleep();
void scheduler_root_update_on_stopped();
void scheduler_root_update_on_wake(int id, int new_priority);

void scheduler_root_assert_active(int id, int unblock_type);
void scheduler_root_deassert_active(int id);
void scheduler_root_set_trace_id(int tid, trace_id_t id);
void scheduler_root_assert_sync(void *args) MCU_ROOT_CODE;
int scheduler_root_unblock_all(void *block_object, int unblock_type);
void scheduler_svcall_set_delaymutex(void *args) MCU_ROOT_EXEC_CODE;

static inline void scheduler_root_set_unblock_type(
  int id,
  scheduler_unblock_type_t unblock_type) MCU_ALWAYS_INLINE;
void scheduler_root_set_unblock_type(int id, scheduler_unblock_type_t unblock_type) {
  sos_sched_table[id].flags &= ~SCHEDULER_TASK_FLAG_UNBLOCK_MASK;
  sos_sched_table[id].flags |= unblock_type;
};

static inline void scheduler_root_assert_waitchild(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_WAITCHILD); }
static inline void scheduler_root_deassert_waitchild(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_WAITCHILD); }
static inline void scheduler_root_assert_inuse(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_INUSE); }
static inline void scheduler_root_deassert_inuse(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_INUSE); }
static inline void scheduler_root_assert_sigcaught(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_SIGCAUGHT); }
static inline void scheduler_root_deassert_sigcaught(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_SIGCAUGHT); }
static inline void scheduler_root_assert_aiosuspend(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_AIOSUSPEND); }
static inline void scheduler_root_deassert_aiosuspend(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_AIOSUSPEND); }
static inline void scheduler_root_assert_listiosuspend(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_LISTIOSUSPEND); }
static inline void scheduler_root_deassert_listiosuspend(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_LISTIOSUSPEND); }

static inline void scheduler_root_assert_cancel(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_CANCEL); }
static inline void scheduler_root_deassert_cancel(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_CANCEL); }
static inline void scheduler_root_assert_cancel_enable(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_CANCEL_ENABLE); }
static inline void scheduler_root_deassert_cancel_enable(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_CANCEL_ENABLE); }
static inline void scheduler_root_assert_cancel_asynchronous(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_CANCEL_ASYNCHRONOUS); }
static inline void scheduler_root_deassert_cancel_asynchronous(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_CANCEL_ASYNCHRONOUS); }

static inline void scheduler_root_assert_authenticated(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_AUTHENTICATED); }
static inline void scheduler_root_deassert_authenticated(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_AUTHENTICATED); }

static inline void scheulder_root_assert_stopped(int id){ task_assert_stopped(id); }
static inline void scheduler_root_deassert_stopped(int id){ task_deassert_stopped(id); }


#endif /* SCHEDULER_SCHEDULER_ROOT_H_ */
