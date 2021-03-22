// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

#include "scheduler_local.h"

void scheduler_svcall_set_delaymutex(void *args) {
  CORTEXM_SVCALL_ENTER();
  sos_sched_table[task_get_current()].signal_delay_mutex = args;
}

void scheduler_root_set_trace_id(int tid, trace_id_t id) {
  sos_sched_table[tid].trace_id = id;
}

void scheduler_root_assert_active(int id, int unblock_type) {
  task_assert_active(id);
  scheduler_root_set_unblock_type(id, unblock_type);
  scheduler_root_deassert_aiosuspend(id);
  // Remove all blocks (mutex, timing, etc)
  sos_sched_table[id].block_object = NULL;
  sos_sched_table[id].wake.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
  sos_sched_table[id].wake.tv_usec = 0;
}

void scheduler_root_deassert_active(int id) {
  task_deassert_active(id);
  task_deassert_exec(id); // stop executing the task
}


void scheduler_root_assert(int id, int flag) { sos_sched_table[id].flags |= (1 << flag); }
void scheduler_root_deassert(int id, int flag) {
  sos_sched_table[id].flags &= ~(1 << flag);
}

/*! @} */
