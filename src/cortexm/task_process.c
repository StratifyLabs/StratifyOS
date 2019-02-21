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

#include <cortexm/task_local.h>
#include <errno.h>
#include "mcu/mcu.h"
#include "mcu/core.h"

int task_create_process(void (*p)(char*),
								void (*cleanup)(void*),
								const char * path_arg,
								task_memories_t * mem,
								void * reent_ptr,
								int parent_id){

	int tid;
	int err;
	void * stackaddr;
	new_task_t task;
	task_memories_t task_memories;
	static int task_process_counter = 1;

	//Variable initialization
	stackaddr = mem->data.addr + mem->data.size;

	//Check the stack alignment
	if ( (unsigned int)stackaddr & 0x03 ){
		errno = EIO;
		return -1;
	}

	//Initialize the task
	task.stackaddr = stackaddr;
	task.start = (uint32_t)p;
	task.stop = (uint32_t)cleanup;
	task.r0 = (uint32_t)path_arg;
	task.r1 = (uint32_t)0;

	task.pid = task_process_counter++; //Assign a new pid
	task.reent = (struct _reent*)reent_ptr;
	task.global_reent = task.reent;

	task.flags = TASK_FLAGS_USED;
	task.parent = task_get_current();
	task.priority = 0;
	task.parent = parent_id;

	memcpy(&task_memories, mem, sizeof(task_memories_t));

	if ( (err = task_mpu_calc_protection(&task_memories)) < 0 ){
		return err;
	}
	task.mem = &task_memories;

	//Do a priv call while accessing the task table so there are no interruptions
	cortexm_svcall( (cortexm_svcall_t)task_root_new_task, &task);
	tid = task.tid;
	return tid;
}
