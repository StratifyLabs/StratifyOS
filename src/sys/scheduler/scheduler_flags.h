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

#ifndef SCHEDULER_SCHEDULER_FLAGS_H_
#define SCHEDULER_SCHEDULER_FLAGS_H_

#include "scheduler_root.h"

static inline int scheduler_sync_asserted(int id){ return sos_sched_table[id].flags & (1<< SCHEDULER_TASK_FLAG_ROOT_SYNC); }
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
static inline void scheduler_root_deassert_sync(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_ROOT_SYNC); }

static inline void scheduler_root_assert_authenticated(int id){ scheduler_root_assert(id, SCHEDULER_TASK_FLAG_AUTHENTICATED); }
static inline void scheduler_root_deassert_authenticated(int id){ scheduler_root_deassert(id, SCHEDULER_TASK_FLAG_AUTHENTICATED); }

static inline void scheulder_root_assert_stopped(int id){ task_assert_stopped(id); }
static inline void scheduler_root_deassert_stopped(int id){ task_deassert_stopped(id); }

#endif /* SCHEDULER_SCHEDULER_FLAGS_H_ */
