// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SYSCALLS
 * @{
 */

/*! \file */

#include "config.h"

#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cortexm/task.h"
#include "mcu/core.h"
#include "mcu/mcu.h"

static void svcall_update_guard(void *args) MCU_ROOT_EXEC_CODE;

// returns zero or returns the previous top of the heap
void *_sbrk_r(struct _reent *reent_ptr, ptrdiff_t incr) {
  char *stack;
  char *base;
  ptrdiff_t size;

  if (reent_ptr->procmem_base == NULL) {
    return NULL;
  }
  size = reent_ptr->procmem_base->size;
  base = (char *)&(reent_ptr->procmem_base->base);

  stack = (void *)task_get_sbrk_stack_ptr(reent_ptr);

  // leave some room for the stack to grow
  if ((stack != NULL) && ((base + size + incr) > (stack - MALLOC_SBRK_JUMP_SIZE * 4))) {
    return NULL;
  }

  // adjust the location of the stack guard -- always 32 bytes for processes
  cortexm_svcall(svcall_update_guard, base + size + incr);

  reent_ptr->procmem_base->size += incr;
  return base + size;
}

void svcall_update_guard(void *args) {
  CORTEXM_SVCALL_ENTER();
  int tid;
  tid = task_get_thread_zero(task_get_pid(task_get_current()));
  task_root_set_stackguard(tid, args, SCHED_DEFAULT_STACKGUARD_SIZE);
}

void *_sbrk(ptrdiff_t incr) { return _sbrk_r(_REENT, incr); }

/*! @} */
