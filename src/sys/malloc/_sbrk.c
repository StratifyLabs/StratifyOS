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

/*! \addtogroup SYSCALLS
 * @{
 */

/*! \file */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <reent.h>

#include "mcu/mcu.h"
#include "cortexm/task.h"
#include "mcu/core.h"


static void svcall_update_guard(void * args) MCU_ROOT_EXEC_CODE;

//returns zero or returns the previous top of the heap
void * _sbrk_r(struct _reent * reent_ptr, ptrdiff_t incr){
	char * stack;
	char * base;
	ptrdiff_t size;

	if ( reent_ptr->procmem_base == NULL ){
		return NULL;
	}
	size = reent_ptr->procmem_base->size;
	base = (char*)&(reent_ptr->procmem_base->base);

	stack = (void*)task_get_sbrk_stack_ptr(reent_ptr);


	//leave some room for the stack to grow
	if ( (stack != NULL) &&  ((base + size + incr) > (stack - MALLOC_SBRK_JUMP_SIZE*4)) ){
		return NULL;
	}

	//adjust the location of the stack guard -- always 32 bytes for processes
	cortexm_svcall(svcall_update_guard, base + size + incr);

	reent_ptr->procmem_base->size += incr;
	return (caddr_t)(base + size);
}

void svcall_update_guard(void * args){
	CORTEXM_SVCALL_ENTER();
	int tid;
	tid = task_get_thread_zero( task_get_pid(task_get_current() ) );
	task_root_set_stackguard(tid, args, SCHED_DEFAULT_STACKGUARD_SIZE);
}


void * _sbrk(ptrdiff_t incr) {
	return _sbrk_r(_REENT, incr);
}

/*! @} */
