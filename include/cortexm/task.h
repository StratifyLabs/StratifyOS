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


#ifndef MCU_TASK_H_
#define MCU_TASK_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "task_types.h"

#ifndef __link

#include "task_table.h"


int task_init(int interval /*! The minimum interval (in ms) between context switches */,
				  void (*scheduler_function)() /*! The scheduler function pointer */,
				  void * system_stack /*! A pointer to the top (highest address) of the system stack */,
				  int system_stack_size /*! The number of bytes used for the system stack */) MCU_ROOT_CODE;

int task_init_mpu(void * system_memory, int system_memory_size) MCU_ROOT_CODE;
int task_validate_memory(void * target, int size);

int task_create_process(void (*p)(char*) /*! The function to execute for the process */,
								void (*cleanup)(void*) /*! The function to call when the process returns */,
								const char * path_arg /*! The path and arguments */,
								task_memories_t * mem /*! The new processes memories */,
								void * reent_ptr /*! A pointer to the reentrancy memory structure */,
								int parent_id /*! Task ID to be assigned as the parent */);


int task_create_thread(void *(*p)(void*)  /*! The function to execute for the task */,
							  void (*cleanup)(void*) /*! The function to call when the thread returns */,
							  void * arg /*! The thread's single argument */,
							  void * mem_addr /*! The address for the bottom of the stack memory */,
							  int mem_size /*! The new threads stack size */,
							  int pid /*! The process ID of the new thread */);

int task_setstackguard(int tid, void * stackaddr, int stacksize);
int task_root_set_stackguard(int tid /*! The task ID */,
									  void * stackaddr /*! The target stack addr (will be aligned to nearest \a stacksize) */,
									  int stacksize /*! The stack size in bytes (must be a power of 2 greater than 16) */) MCU_ROOT_CODE;


u8 task_get_exec_count();
u64 task_gettime(int tid);
u64 task_root_gettime(int tid) MCU_ROOT_CODE;

void task_root_delete(int id /*! The task to delete */) MCU_ROOT_CODE;

void task_root_switch_context() MCU_ROOT_CODE;
void task_root_resetstack(int id) MCU_ROOT_CODE;


void * task_get_sbrk_stack_ptr(struct _reent * reent_ptr);


int task_get_thread_zero(int pid /*! The process ID */);

int task_init_single(int (*initial_thread)() /*! the single thread to execute */,
							void * system_memory /*! The location of the system memory */,
							int system_memory_size /*! The size of the system memory */) MCU_ROOT_CODE;

int task_interrupt(task_interrupt_t * intr);
void task_root_interrupt(void * args) MCU_ROOT_CODE;
int task_mpu_calc_protection(task_memories_t * mem);
u32 task_interrupt_stacksize();

s8 task_get_current_priority();
void task_root_set_current_priority(s8 value);
void task_root_elevate_current_priority(s8 value);

//higher numbers take precedence over lower numbers
#define TASK_SYSTEM_STACK_MPU_REGION 7
#define TASK_SYSTEM_CODE_MPU_REGION 6
#define TASK_SYSTEM_DATA_MPU_REGION 5
#define TASK_APPLICATION_STACK_GUARD_REGION 4 //stack guard must have higher precedence than application data
#define TASK_APPLICATION_CODE_MPU_REGION 2
#define TASK_APPLICATION_DATA_MPU_REGION 3
#define TASK_APPLICATION_DATA_USER_REGION_HIGH_PRIORITY 1
#define TASK_APPLICATION_DATA_USER_REGION_LOW_PRIORITY 0


#endif

#ifdef __cplusplus
}
#endif


#endif // MCU_TASK_H_

