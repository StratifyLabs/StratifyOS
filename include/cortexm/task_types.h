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

#ifndef CORTEXM_TASK_TYPES_H_
#define CORTEXM_TASK_TYPES_H_

#include "mcu/arch.h"

#if !defined ARCH_DEFINED
#error "Architecture is not yet defined"
#endif

typedef union {
	volatile u32 t_atomic[2] /*! Word accessible task timer */;
	volatile u64 t /*! Task timer */;
} task_timer_t;


typedef struct {
	void * address /*! The address of the memory as input, converted to RBAR after process starts */;
	u32 size /*! The size of the memory as input, converted to RASR after task starts */;
	u32 rbar;
	u32 rasr;
} task_memory_t;


typedef struct {
	task_memory_t code /*! Executable code memory */;
	task_memory_t data /*! Data memory */;
	task_memory_t stackguard /*! The task stack guard */;
} task_memories_t;

typedef struct {
	void (*p)(int,char * const*) /*! process start function */;
	int argc /*! The number of arguments */;
	char * const * argv /*! Arguments */;
	task_memories_t memories /*! Memories */;
	void * reent_ptr /*! Location of re-entrancy structure */;
} task_process_t;


typedef struct {
	void *(*p)(void*) /*! The thread start function */;
	void * arg /*! The argument to the thread */;
	void * mem_base /*! A pointer to the stack memory base */;
	int mem_size /*! The number of bytes avaiable for the stack */;
	int pid /*! The process ID assigned to the new thread */;
} task_thread_t;


typedef struct MCU_PACK {
	volatile void * sp /*! The task stack pointer */;
	int pid /*! The process id */;
	volatile s8 priority /*! Task priority */;
	volatile u8 flags /*! Status flags */;
	volatile u16 parent /*! Parent process ID */;
	volatile task_timer_t timer /*! The task timer */;
	task_memories_t mem /*! The task memories */;
	void * global_reent /*! Points to process re-entrancy data */;
	void * reent /*! Points to thread re-entrancy data */;
	int rr_time /*! The amount of time the task used in the round robin */;
#if __FPU_USED == 1
	u32 fp[32];
	u32 fpscr;
#endif
} task_t;


typedef void (*task_interrupt_handler_t)(int,int,int,int);

typedef struct {
	int tid;
	task_interrupt_handler_t handler;
	void (*sync_callback)(void*);
	void * sync_callback_arg;
	int arg[4];
} task_interrupt_t;



#endif /* CORTEXM_TASK_TYPES_H_ */
