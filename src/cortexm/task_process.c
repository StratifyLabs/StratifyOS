// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>

#include "task_local.h"

int task_create_process(
  void (*p)(char *),
  void (*cleanup)(void *),
  const char *path_arg,
  task_memories_t *mem,
  void *reent_ptr,
  int parent_id) {

  int tid;
  int err;
  void *stackaddr;
  new_task_t task;
  task_memories_t task_memories;
  static int task_process_counter = 1;

  // Variable initialization
  stackaddr = mem->data.address + mem->data.size;

  // Check the stack alignment
  if ((unsigned int)stackaddr & 0x03) {
    errno = EIO;
    return -1;
  }

  // Initialize the task
  task.stackaddr = stackaddr;
  task.start = (uint32_t)p;
  task.stop = (uint32_t)cleanup;
  task.r0 = (uint32_t)path_arg;
  task.r1 = (uint32_t)0;

  task.pid = task_process_counter++; // Assign a new pid
  task.reent = (struct _reent *)reent_ptr;
  task.global_reent = task.reent;

  task.flags = TASK_FLAGS_USED;
  task.priority = 0;
  task.parent = parent_id;

  task_memories = *mem;
  if ((err = task_mpu_calc_protection(&task_memories)) < 0) {
    return err;
  }
  task.mem = &task_memories;

  // Do a priv call while accessing the task table so there are no interruptions
  cortexm_svcall((cortexm_svcall_t)task_svcall_new_task, &task);
  tid = task.tid;
  return tid;
}
