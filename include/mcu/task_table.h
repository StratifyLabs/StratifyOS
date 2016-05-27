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

#ifndef TASK_TABLE_H_
#define TASK_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __link

#define TASK_FLAGS_USED (1<<8)
#define TASK_FLAGS_EXEC (1<<9)
#define TASK_FLAGS_PRIV (1<<10)
#define TASK_FLAGS_IS_THREAD (1<<11)
#define TASK_FLAGS_IS_FIFO (1<<12)
#define TASK_FLAGS_YIELD (1<<13)
#define TASK_FLAGS_ROOT (1<<14)
#define TASK_FLAGS_PARENT(x) ( (x & 0xFFFF) << 16 )

#define TASK_FLAGS_GET_PARENT(flags) ( flags >> 16 )
#define TASK_FLAGS_PARENT_MASK 0xFFFF0000


extern task_t * task_table;
extern int task_total;
extern int task_rr_reload;
extern volatile task_timer_t task_clock;
extern volatile int task_current;

static inline int task_get_rr_reload() MCU_ALWAYS_INLINE;
int task_get_rr_reload(){
	return task_rr_reload;
}

static inline void task_set_reent(int id, struct _reent * reent, struct _reent * global_reent) MCU_ALWAYS_INLINE;
void task_set_reent(int id, struct _reent * reent, struct _reent * global_reent){
	task_table[id].reent = reent;
	task_table[id].global_reent = global_reent;
}

static inline void task_deassert_exec(int id) MCU_ALWAYS_INLINE;
void task_deassert_exec(int id){
	task_table[id].flags &= ~(TASK_FLAGS_EXEC);
}

static inline void task_assert_exec(int id) MCU_ALWAYS_INLINE;
void task_assert_exec(int id){
	task_table[id].flags |= (TASK_FLAGS_EXEC);
}

static inline int task_exec_asserted(int id) MCU_ALWAYS_INLINE;
int task_exec_asserted(int id){
	return ( (task_table[id].flags & (TASK_FLAGS_EXEC)) ==  TASK_FLAGS_EXEC);
}

static inline void task_deassert_root(int id) MCU_ALWAYS_INLINE;
void task_deassert_root(int id){
	task_table[id].flags &= ~(TASK_FLAGS_ROOT);
}

static inline void task_assert_root(int id) MCU_ALWAYS_INLINE;
void task_assert_root(int id){
	task_table[id].flags |= (TASK_FLAGS_ROOT);
}

static inline int task_root_asserted(int id) MCU_ALWAYS_INLINE;
int task_root_asserted(int id){
	return ( (task_table[id].flags & (TASK_FLAGS_ROOT)) ==  TASK_FLAGS_ROOT);
}

static inline void task_deassert_yield(int id) MCU_ALWAYS_INLINE;
void task_deassert_yield(int id){
	task_table[id].flags &= ~(TASK_FLAGS_YIELD);
}

static inline void task_assert_yield(int id) MCU_ALWAYS_INLINE;
void task_assert_yield(int id){
	task_table[id].flags |= (TASK_FLAGS_YIELD);
}

static inline int task_yield_asserted(int id) MCU_ALWAYS_INLINE;
int task_yield_asserted(int id){
	return ( (task_table[id].flags & (TASK_FLAGS_YIELD)) ==  TASK_FLAGS_YIELD);
}

static inline int task_isthread_asserted(int id) MCU_ALWAYS_INLINE;
int task_isthread_asserted(int id){
	return ( (task_table[id].flags & (TASK_FLAGS_IS_THREAD)) ==  TASK_FLAGS_IS_THREAD);
}

static inline void task_deassert_priv(int id) MCU_ALWAYS_INLINE;
void task_deassert_priv(int id){
	task_table[id].flags &= ~(TASK_FLAGS_PRIV);
}

static inline void task_assert_priv(int id) MCU_ALWAYS_INLINE;
void task_assert_priv(int id){
	task_table[id].flags |= (TASK_FLAGS_PRIV);
}

static inline int task_priv_asserted(int id) MCU_ALWAYS_INLINE;
int task_priv_asserted(int id){
	return ( (task_table[id].flags & (TASK_FLAGS_PRIV)) ==  TASK_FLAGS_PRIV);
}

static inline void task_deassert_isfifo(int id) MCU_ALWAYS_INLINE;
void task_deassert_isfifo(int id){
	task_table[id].flags &= ~(TASK_FLAGS_IS_FIFO);
}

static inline void task_assert_isfifo(int id) MCU_ALWAYS_INLINE;
void task_assert_isfifo(int id){
	task_table[id].flags |= (TASK_FLAGS_IS_FIFO);
}

static inline int task_isfifo_asserted(int id) MCU_ALWAYS_INLINE;
int task_isfifo_asserted(int id){
	return ( (task_table[id].flags & (TASK_FLAGS_IS_FIFO)) ==  TASK_FLAGS_IS_FIFO);
}


static inline void task_set_parent(int id, int parent) MCU_ALWAYS_INLINE;
void task_set_parent(int id, int parent){
	task_table[id].flags &= ~TASK_FLAGS_PARENT_MASK;
	task_table[id].flags |= parent;
}

static inline int task_get_parent(int id) MCU_ALWAYS_INLINE;
int task_get_parent(int id){
	return TASK_FLAGS_GET_PARENT(task_table[id].flags);
}

static inline int task_used_asserted(int id) MCU_ALWAYS_INLINE;
int task_used_asserted(int id){
	return ((task_table[id].flags & (TASK_FLAGS_USED)) == TASK_FLAGS_USED);
}

static inline void task_assert_used(int id) MCU_ALWAYS_INLINE;
void task_assert_used(int id){
	task_table[id].flags |= (TASK_FLAGS_USED);
}

static inline void task_deassert_used(int id) MCU_ALWAYS_INLINE;
void task_deassert_used(int id){
	task_table[id].flags &= ~(TASK_FLAGS_USED);
}

static inline int task_enabled(int id) MCU_ALWAYS_INLINE;
int task_enabled(int id){
	return ((task_table[id].flags & (TASK_FLAGS_USED)) == TASK_FLAGS_USED);
}

static inline int task_get_current() MCU_ALWAYS_INLINE;
int task_get_current(){
	return task_current;
}

static inline int task_get_pid(int id) MCU_ALWAYS_INLINE;
int task_get_pid(int id){
	return task_table[id].pid;
}

static inline int task_get_total() MCU_ALWAYS_INLINE;
int task_get_total(){
	return task_total;
}

static inline void task_get_timer(uint32_t * dest, int id) MCU_ALWAYS_INLINE;
void task_get_timer(uint32_t * dest, int id){
	dest[1] = task_table[id].timer.t_atomic[1];
	dest[0] = task_table[id].timer.t_atomic[0];
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* TASK_TABLE_H_ */
