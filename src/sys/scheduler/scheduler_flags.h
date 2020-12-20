// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef SCHEDULER_SCHEDULER_FLAGS_H_
#define SCHEDULER_SCHEDULER_FLAGS_H_

#include "scheduler_root.h"

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

#endif /* SCHEDULER_SCHEDULER_FLAGS_H_ */
