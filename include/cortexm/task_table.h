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

#ifndef TASK_TABLE_H_
#define TASK_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __link

//SCHEDULER_TASK_FLAG_ACTIVE
//#define SCHEDULER_TASK_FLAG_STOPPED 11

//flags 0 to 7 are unused
#define TASK_FLAGS_USED (1<<0) //task is currently being used
#define TASK_FLAGS_EXEC (1<<1) //set if task is part of current tasks being executed (highest priority of active tasks)
#define TASK_FLAGS_ACTIVE (1<<2) //Task is currently active (it is not blocked or sleeping)
#define TASK_FLAGS_THREAD (1<<3) //Task is a thread task rather than a process (first thread)
#define TASK_FLAGS_FIFO (1<<4) //Task is executed in FIFO rather than Round Robin mode
#define TASK_FLAGS_STOPPED (1<<5) //Task has been stopped by a signal
#define TASK_FLAGS_ROOT (1<<6) //Task has root privileges
#define TASK_FLAGS_YIELD (1<<7) //current task wants to yield the processor -- also used internally by context switcher to track SVCALL

extern volatile task_t sos_task_table[];

static inline int task_enabled_active_not_stopped(int id){
    return (sos_task_table[id].flags & (TASK_FLAGS_USED | TASK_FLAGS_ACTIVE | TASK_FLAGS_STOPPED)) == (TASK_FLAGS_ACTIVE | TASK_FLAGS_USED );
}

static inline int task_enabled_not_active(int id){
    return (sos_task_table[id].flags & (TASK_FLAGS_USED | TASK_FLAGS_ACTIVE)) == (TASK_FLAGS_USED );
}

static inline void task_assert_flag(int id, u8 flag){ sos_task_table[id].flags |= flag; }
static inline void task_deassert_flag(int id, u8 flag){ sos_task_table[id].flags &= ~flag; }
static inline int task_flag_asserted(int id, u8 flag){
    return ( (sos_task_table[id].flags & (flag)) ==  flag);
}

static inline void task_set_reent(int id, struct _reent * reent, struct _reent * global_reent){
    sos_task_table[id].reent = reent;
    sos_task_table[id].global_reent = global_reent;
}

static inline void task_assert_used(int id){ task_assert_flag(id, TASK_FLAGS_USED); }
static inline void task_deassert_used(int id){ task_deassert_flag(id, TASK_FLAGS_USED); }
static inline int task_used_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_USED); }
static inline int task_enabled(int id){ return task_flag_asserted(id, TASK_FLAGS_USED); }

static inline void task_assert_exec(int id){ task_assert_flag(id, TASK_FLAGS_EXEC); }
static inline void task_deassert_exec(int id){ task_deassert_flag(id, TASK_FLAGS_EXEC); }
static inline int task_exec_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_EXEC); }

static inline void task_assert_active(int id){ task_assert_flag(id, TASK_FLAGS_ACTIVE); }
static inline void task_deassert_active(int id){ task_deassert_flag(id, TASK_FLAGS_ACTIVE); }
static inline int task_active_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_ACTIVE); }

static inline void task_assert_thread(int id){ task_assert_flag(id, TASK_FLAGS_THREAD); }
static inline void task_deassert_thread(int id){ task_deassert_flag(id, TASK_FLAGS_THREAD); }
static inline int task_thread_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_THREAD); }

static inline void task_assert_fifo(int id){ task_assert_flag(id, TASK_FLAGS_FIFO); }
static inline void task_deassert_fifo(int id){ task_deassert_flag(id, TASK_FLAGS_FIFO); }
static inline int task_fifo_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_FIFO); }

static inline void task_assert_stopped(int id){ task_assert_flag(id, TASK_FLAGS_STOPPED); }
static inline void task_deassert_stopped(int id){ task_deassert_flag(id, TASK_FLAGS_STOPPED); }
static inline int task_stopped_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_STOPPED); }

static inline void task_assert_root(int id){ task_assert_flag(id, TASK_FLAGS_ROOT); }
static inline void task_deassert_root(int id){ task_deassert_flag(id, TASK_FLAGS_ROOT); }
static inline int task_root_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_ROOT); }

static inline void task_assert_yield(int id){ task_assert_flag(id, TASK_FLAGS_YIELD); }
static inline void task_deassert_yield(int id){ task_deassert_flag(id, TASK_FLAGS_YIELD); }
static inline int task_yield_asserted(int id){ return task_flag_asserted(id, TASK_FLAGS_YIELD); }

static inline void task_set_parent(int id, int parent){ sos_task_table[id].parent = parent; }
static inline int task_get_parent(int id){ return sos_task_table[id].parent; }
static inline void task_set_priority(int id, int priority){ sos_task_table[id].priority = priority; }
static inline s8 task_get_priority(int id){ return sos_task_table[id].priority; }

extern volatile int m_task_current;
static inline int task_get_current(){ return m_task_current; }

static inline int task_get_pid(int id);
int task_get_pid(int id){
    return sos_task_table[id].pid;
}

u8 task_get_total();

static inline void task_get_timer(u32 * dest, int id){
    dest[1] = sos_task_table[id].timer.t_atomic[1];
    dest[0] = sos_task_table[id].timer.t_atomic[0];
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* TASK_TABLE_H_ */
