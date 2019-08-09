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

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

#include "scheduler_local.h"

void scheduler_svcall_set_delaymutex(void * args){
	CORTEXM_SVCALL_ENTER();
	sos_sched_table[ task_get_current() ].signal_delay_mutex = args;
}

void scheduler_root_assert_sync(void * args){
	//verify the calling PC is correct?

	sos_sched_table[task_get_current()].flags |= (1<< SCHEDULER_TASK_FLAG_ROOT_SYNC);
}

void scheduler_root_set_trace_id(int tid, trace_id_t id){
	sos_sched_table[tid].trace_id = id;
}

void scheduler_root_assert_active(int id, int unblock_type){
	task_assert_active(id);
	scheduler_root_set_unblock_type(id, unblock_type);
	scheduler_root_deassert_aiosuspend(id);
	//Remove all blocks (mutex, timing, etc)
	sos_sched_table[id].block_object = NULL;
	sos_sched_table[id].wake.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
	sos_sched_table[id].wake.tv_usec = 0;
}

void scheduler_root_deassert_active(int id){
	task_deassert_active(id);
	task_deassert_exec(id);  //stop executing the task
}

void scheduler_root_stop_task(int id){
	scheduler_root_deassert_active(id);
}

void scheulder_root_start_task(int id){
	scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_SIGNAL);
}

void scheduler_root_assert(int id, int flag){
	sos_sched_table[id].flags |= (1<<flag);

}
void scheduler_root_deassert(int id, int flag){
	sos_sched_table[id].flags &= ~(1<<flag);
}




/*! @} */
