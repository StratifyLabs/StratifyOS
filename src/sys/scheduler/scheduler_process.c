// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

#include "config.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#include "scheduler_root.h"
#include "scheduler_timing.h"

typedef struct {
  task_memories_t *mem;
  int tid;
  int is_authenticated;
} init_sched_task_t;

static void svcall_init_sched_task(init_sched_task_t *task) MCU_ROOT_EXEC_CODE;
static void cleanup_process(void *status);

/*! \details This function creates a new process.
 * \return The thread id or zero if the thread could not be created.
 */
int scheduler_create_process(
  void (*p)(char *) /*! The startup function (crt()) */,
  const char *path_arg /*! Path string with arguments */,
  task_memories_t *mem,
  void *reent /*! The location of the reent structure */,
  int parent_id,
  int is_authenticated) {
  int tid;
  init_sched_task_t args;

  // Start a new process
  tid = task_create_process(p, cleanup_process, path_arg, mem, reent, parent_id);

  if (tid > 0) {
    // update the scheduler table using a privileged call
    args.tid = tid;
    args.mem = mem;
    args.is_authenticated = is_authenticated;
    cortexm_svcall((cortexm_svcall_t)svcall_init_sched_task, &args);
  } else {
    return -1;
  }

#if SOS_DEBUG
  const pid_t pid = task_get_pid(tid);
  sos_debug_log_info(
    SOS_DEBUG_SYS, "start process id:%d pid:%d parent:%d (%d)", tid, pid,
    task_get_parent(tid), task_get_current());

  sos_debug_log_directive(
    SOS_DEBUG_MALLOC, "heap:Process %d Heap:heap%d:Process %d Heap Utilization over time",
    pid, pid, pid);

#endif

  return task_get_pid(tid);
}

void svcall_init_sched_task(init_sched_task_t *task) {
  CORTEXM_SVCALL_ENTER();
  int id = task->tid;

  memset((void *)&sos_sched_table[id], 0, sizeof(sched_task_t));

  PTHREAD_ATTR_SET_IS_INITIALIZED((&(sos_sched_table[id].attr)), 1);
  PTHREAD_ATTR_SET_SCHED_POLICY((&(sos_sched_table[id].attr)), SCHED_OTHER);
  PTHREAD_ATTR_SET_GUARDSIZE(
    (&(sos_sched_table[id].attr)), CONFIG_TASK_DEFAULT_STACKGUARD_SIZE);
  PTHREAD_ATTR_SET_CONTENTION_SCOPE((&(sos_sched_table[id].attr)), PTHREAD_SCOPE_SYSTEM);
  PTHREAD_ATTR_SET_INHERIT_SCHED((&(sos_sched_table[id].attr)), PTHREAD_EXPLICIT_SCHED);
  PTHREAD_ATTR_SET_DETACH_STATE((&(sos_sched_table[id].attr)), PTHREAD_CREATE_DETACHED);

  if (task->is_authenticated && scheduler_authenticated_asserted(task_get_current())) {
    scheduler_root_assert_authenticated(id);
  }

  sos_sched_table[id].attr.stackaddr =
    task->mem->data.address; // Beginning of process data memory
  sos_sched_table[id].attr.stacksize =
    task->mem->data.size; // Size of the memory (not just the stack)
  sos_sched_table[id].attr.schedparam.sched_priority =
    0; // This is the priority to revert to after being escalated

  scheduler_timing_root_process_timer_initialize(id);

  sos_sched_table[id].wake.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
  sos_sched_table[id].wake.tv_usec = 0;
  scheduler_root_assert_cancel_enable(id);
  scheduler_root_deassert_cancel_asynchronous(id);
  scheduler_root_assert_active(id, 0);
  scheduler_root_assert_inuse(id);
  scheduler_root_update_on_wake(id, task_get_priority(id));
  u32 stackguard = (u32)task->mem->data.address + sizeof(struct _reent);
  if (
    task_root_set_stackguard(id, (void *)stackguard, CONFIG_TASK_DEFAULT_STACKGUARD_SIZE) < 0) {
    sos_debug_log_warning(SOS_DEBUG_SCHEDULER, "Failed to set stackguard");
  }

  // Items inherited from parent process

  // Signal mask
  struct _reent *const reent = (struct _reent *)sos_task_table[id].reent;
  reent->sigmask = _REENT->sigmask;
}

static void cleanup_process(void *status) {
  // Processes should ALWAYS use exit -- this should never get called but is here just in
  // case
  SOS_TRACE_FATAL("cleanup");
  kill(task_get_pid(task_get_current()), SIGKILL);
}
