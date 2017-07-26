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

/*! \addtogroup TASK Task Management
 * @{
 *
 *
 * \ingroup CORE
 *
 * \details The Task Management module is an API to manage tasks including processes and threads.  A
 * process is defined as a set of threads.  Processor time is given to each thread that has its
 * exec flag set (see task_assert_exec() and task_deassert_exec()).
 *
 *
 */

/*! \file
 * \brief Task Management Header File
 *
 */

#ifndef MCU_TASK_H_
#define MCU_TASK_H_

#include <stdint.h>
#include "cortexm/cortexm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Defines a 64-bit task timer.
 * \details This structure defines a union for a 64-bit task timer.
 *
 */
typedef union {
	volatile u32 t_atomic[2] /*! Word accessible task timer */;
	volatile u64 t /*! Task timer */;
} task_timer_t;

/*! \details This structure defines a section of
 * task memory.
 *
 */
typedef struct {
	void * addr /*! The address of the memory */;
	u32 size /*! The size of the memory */;
} task_memory_t;

/*! \details This data structure stores the
 * memories used by a process.
 */
typedef struct {
	task_memory_t code /*! Executable code memory */;
	task_memory_t data /*! Data memory */;
	task_memory_t stackguard /*! The task stack guard */;
} task_memories_t;


/*! \details This structure contains the data required
 * to start a new process.
 *
 */
typedef struct {
	void (*p)(int,char * const*) /*! process start function */;
	int argc /*! The number of arguments */;
	char * const * argv /*! Arguments */;
	task_memories_t memories /*! Memories */;
	void * reent_ptr /*! Location of re-entrancy structure */;
} task_process_t;

/*! \details This structure contains the data required
 * to start a new thread.
 */
typedef struct {
	void *(*p)(void*) /*! The thread start function */;
	void * arg /*! The argument to the thread */;
	void * mem_base /*! A pointer to the stack memory base */;
	int mem_size /*! The number of bytes avaiable for the stack */;
	int pid /*! The process ID assigned to the new thread */;
} task_thread_t;

/*! \details This structure contains
 * the data required for one task.  It constitutes an entry
 * in the task table.
 */
typedef struct {
	volatile void * sp /*! The task stack pointer */;
	int pid /*! The process id */;
	volatile int flags /*! Status flags */;
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

#ifndef __link

#include "task_table.h"

/*! \brief Initializes the system for multithreaded applications.
 * \details This function initialize the task context switcher.  After this function
 * has been called, task_new_process() or task_new_thread() can be used to create new tasks.
 */
int task_init(int interval /*! The minimum interval (in ms) between context switches */,
		void (*scheduler_function)() /*! The scheduler function pointer */,
		void * system_stack /*! A pointer to the top (highest address) of the system stack */,
		int system_stack_size /*! The number of bytes used for the system stack */) MCU_PRIV_CODE;


int task_init_mpu(void * system_memory, int system_memory_size) MCU_PRIV_CODE;

/*! \details This function creates a new task.
 * \return The task id (id)
 */
int task_new_process(void (*p)(char*) /*! The function to execute for the process */,
		void (*cleanup)(void*) /*! The function to call when the process returns */,
		const char * path_arg /*! The path and arguments */,
		task_memories_t * mem /*! The new processes memories */,
		void * reent_ptr /*! A pointer to the reentrancy memory structure */);

/*! \details This function creates a new thread.
 * \return The thread ID or zero if the task could not be created.
 */
int task_new_thread(void *(*p)(void*)  /*! The function to execute for the task */,
		void (*cleanup)(void*) /*! The function to call when the thread returns */,
		void * arg /*! The thread's single argument */,
		void * mem_addr /*! The address for the bottom of the stack memory */,
		int mem_size /*! The new threads stack size */,
		int pid /*! The process ID of the new thread */);

/*! \details This function configures
 * the stack guard for use with the memory protection unit.
 * \return Zero on success
 */
int task_root_set_stackguard(int tid /*! The task ID */,
		void * stackaddr /*! The target stack addr (will be aligned to nearest \a stacksize) */,
		int stacksize /*! The stack size in bytes (must be a power of 2 greater than 16) */) MCU_PRIV_CODE;


int task_setstackguard(int tid, void * stackaddr, int stacksize);

void task_reload();


/*! \details This function gets the amount of time in CPU clocks that the specified task
 * has used.
 * \return The amount of time spent on the task.
 */
u64 task_gettime(int tid);
u64 task_root_gettime(int tid) MCU_PRIV_CODE;

/*! \details This function sets the context switching interval.
 * \return The actual interval.
 */
int task_set_interval(int interval /*! The context switching interval in ms */) MCU_PRIV_CODE;

/*! \details This function kills the specified task.
 * \return an HWPL error code
 */
void task_root_del(int id /*! The task to delete */) MCU_PRIV_CODE;

void task_root_switch_context(void*) MCU_PRIV_CODE;


void task_root_resetstack(int id) MCU_PRIV_CODE;



/*! \details This function returns the stack pointer
 * of for the memory where malloc is being allocated.
 */
void * task_get_sbrk_stack_ptr(struct _reent * reent_ptr);



/*! \details This function gets the task associated with thread 0 of the
 * process.
 */
int task_get_thread_zero(int pid /*! The process ID */);


/*! \details This function initializes the task system if only one task
 * is to be used.
 *
 * \return 0
 */
int task_init_single(int (*initial_thread)() /*! the single thread to execute */,
		void * system_memory /*! The location of the system memory */,
		int system_memory_size /*! The size of the system memory */) MCU_PRIV_CODE;

int task_interrupt(task_interrupt_t * intr);
void task_root_interrupt(void * args) MCU_PRIV_CODE;
int task_mpu_calc_protection(task_memories_t * mem);
u32 task_interrupt_stacksize();

#endif

#ifdef __cplusplus
}
#endif


#endif // MCU_TASK_H_

/*! @} */
