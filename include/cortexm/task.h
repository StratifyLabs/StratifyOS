// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_TASK_H_
#define MCU_TASK_H_

#include <sdk/types.h>

#include "sos/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "task_types.h"

#ifndef __link

#include "task_table.h"

int task_init(
  int interval /*! The minimum interval (in ms) between context switches */,
  void (*scheduler_function)() /*! The scheduler function pointer */,
  void *system_stack /*! A pointer to the top (highest address) of the system stack */,
  int system_stack_size /*! The number of bytes used for the system stack */)
  MCU_ROOT_CODE;

int task_init_mpu(void *system_memory, int system_memory_size) MCU_ROOT_CODE;
int task_validate_memory(void *target, int size) MCU_ROOT_CODE;

int task_create_process(
  void (*p)(char *) /*! The function to execute for the process */,
  void (*cleanup)(void *) /*! The function to call when the process returns */,
  const char *path_arg /*! The path and arguments */,
  task_memories_t *mem /*! The new processes memories */,
  void *reent_ptr /*! A pointer to the reentrancy memory structure */,
  int parent_id /*! Task ID to be assigned as the parent */);

int task_create_thread(
  void *(*p)(void *)/*! The function to execute for the task */,
  void (*cleanup)(void *) /*! The function to call when the thread returns */,
  void *arg /*! The thread's single argument */,
  void *mem_addr /*! The address for the bottom of the stack memory */,
  int mem_size /*! The new threads stack size */,
  int pid /*! The process ID of the new thread */);

int task_setstackguard(int tid, void *stackaddr, int stacksize);
int task_root_set_stackguard(
  int tid /*! The task ID */,
  void *stackaddr /*! The target stack addr (will be aligned to nearest \a stacksize) */,
  int stacksize /*! The stack size in bytes (must be a power of 2 greater than 16) */)
  MCU_ROOT_CODE;

u64 task_gettime(int tid);
u64 task_root_gettime(int tid) MCU_ROOT_CODE;

void task_root_delete(int id /*! The task to delete */) MCU_ROOT_CODE;

void task_root_switch_context() MCU_ROOT_CODE;
void task_root_resetstack(int id) MCU_ROOT_CODE;

void *task_get_sbrk_stack_ptr(struct _reent *reent_ptr);

int task_get_thread_zero(int pid /*! The process ID */);

int task_init_single(
  int (*initial_thread)() /*! the single thread to execute */,
  void *system_memory /*! The location of the system memory */,
  int system_memory_size /*! The size of the system memory */) MCU_ROOT_CODE;

int task_interrupt(task_interrupt_t *intr);
void task_svcall_interrupt(void *args) MCU_ROOT_CODE;
int task_mpu_calc_protection(task_memories_t *mem);
u32 task_interrupt_stacksize();

u8 task_get_exec_count();
u8 task_get_total();

extern volatile s8 m_task_current_priority MCU_SYS_MEM;
static inline s8 task_get_current_priority() { return m_task_current_priority; }
static inline void task_root_set_current_priority(s8 value) {
  m_task_current_priority = value;
}

void task_root_elevate_current_priority(s8 value) MCU_ROOT_EXEC_CODE;

u32 task_reverse_memory_lookup(u32 input);



// weak so bootloader can override
void cortexm_systick_handler() MCU_NAKED MCU_ROOT_EXEC_CODE MCU_WEAK;
void cortexm_pendsv_handler() MCU_NAKED MCU_ROOT_EXEC_CODE MCU_WEAK;

#endif

#ifdef __cplusplus
}
#endif

#endif // MCU_TASK_H_
