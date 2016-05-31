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

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

#include "sched_flags.h"

void sched_priv_set_delaymutex(void * args){
	stratify_sched_table[ task_get_current() ].signal_delay_mutex = args;
}

void sched_priv_assert_sync(void * args){
	//verify the calling PC is correct?

	stratify_sched_table[task_get_current()].flags |= (1<< SCHED_TASK_FLAGS_PRIV_SYNC);
}

void sched_priv_set_trace_id(int tid, trace_id_t id){
	stratify_sched_table[tid].trace_id = id;
}

void sched_priv_assert_active(int id, int unblock_type){
	stratify_sched_table[id].flags |= (1<< SCHED_TASK_FLAGS_ACTIVE);
	sched_priv_set_unblock_type(id, unblock_type);
	sched_priv_deassert_aiosuspend(id);
	//Remove all blocks (mutex, timing, etc)
	stratify_sched_table[id].block_object = NULL;
	stratify_sched_table[id].wake.tv_sec = SCHED_TIMEVAL_SEC_INVALID;
	stratify_sched_table[id].wake.tv_usec = 0;
}

void sched_priv_deassert_active(int id){
	stratify_sched_table[id].flags &= ~(1<< SCHED_TASK_FLAGS_ACTIVE);
	task_deassert_exec(id);  //stop executing the task
	sched_priv_assert_status_change(); //notify the scheduler a task has changed status
}

void sched_priv_assert_status_change(){
	sched_status_changed = 1;
}

inline void sched_priv_deassert_status_change(){
	sched_status_changed = 0;
}

void sched_priv_stop_task(int id){
	sched_priv_deassert_active(id);
}

void sched_priv_start_task(int id){
	sched_priv_assert_active(id, SCHED_UNBLOCK_SIGNAL);
}

void sched_priv_assert(int id, int flag){
	stratify_sched_table[id].flags |= (1<<flag);

}
void sched_priv_deassert(int id, int flag){
	stratify_sched_table[id].flags &= ~(1<<flag);
}




/*! @} */
