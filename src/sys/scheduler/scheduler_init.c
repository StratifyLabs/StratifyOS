// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */
#include "config.h"
#include "scheduler_local.h"

#include "sos/debug.h"
#include "sos/symbols.h"

volatile scheduler_fault_t m_scheduler_fault MCU_SYS_MEM;

void scheduler_init() {
  memset((void *)sos_task_table, 0, sizeof(task_t) * sos_config.task.task_total);
  memset((void *)sos_sched_table, 0, sizeof(sched_task_t) * sos_config.task.task_total);

  // Do basic init of task 0 so that memory allocation can happen before the scheduler
  // starts
  sos_task_table[0].reent = _impure_ptr;
  sos_task_table[0].global_reent = _global_impure_ptr;
}

void scheduler_start(void *(*init)(void *)) {
  sos_sched_table[0].init = init;
  sos_sched_table[0].attr.stackaddr = &_data;
  sos_sched_table[0].attr.stacksize = sos_config.sys.memory_size;

  task_init(
    SCHED_RR_DURATION,
    scheduler, // run the scheduler
    // Let the task init function figure out where the stack needs to be
    NULL, sos_config.sys.memory_size);

  sos_handle_event(SOS_EVENT_ROOT_FATAL, "scheduler failed");
}

static void svcall_prepare(void *args) {
  sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Init MPU");
  if (task_init_mpu(&_data, sos_config.sys.memory_size) < 0) {
    sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Failed to initialize memory protection");
    sos_handle_event(SOS_EVENT_ROOT_FATAL, (void *)"tski");
  }
  sos_handle_event(SOS_EVENT_ROOT_DEBUG_INITIALIZED, 0);
}

void scheduler_prepare() {
  cortexm_svcall(svcall_prepare, NULL);
  scheduler_timing_init();
  cortexm_set_unprivileged_mode(); // Enter unpriv mode
}

/*! @} */
