// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */
#include "scheduler_fault.h"
#include "scheduler_timing.h"

#include "cortexm/fault_local.h"
#include "sos/debug.h"
#include "sos/symbols.h"

volatile sched_task_t sos_sched_table[CONFIG_TASK_TOTAL] MCU_SYS_MEM;

static void init_mpu() {
  sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Init MPU");
  m_cortexm_fault_handler.callback = scheduler_root_fault_handler;
  m_cortexm_fault_handler.context = NULL;

  if (task_init_mpu(&_data, sos_config.sys.memory_size) < 0) {
    sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Failed to initialize memory protection");
    sos_handle_event(SOS_EVENT_ROOT_FATAL, (void *)"scheduler init mpu");
  }
  sos_handle_event(SOS_EVENT_ROOT_DEBUG_INITIALIZED, 0);
}

void scheduler_init() {
  for(int i=0; i < CONFIG_TASK_TOTAL; i++){
    sos_task_table[i] = (task_t){};
    sos_sched_table[i] = (sched_task_t){};
  }

  // Do basic init of task 0 so that memory allocation can happen before the scheduler
  // starts
  sos_task_table[0].reent = _impure_ptr;
  sos_task_table[0].global_reent = _global_impure_ptr;
}

void scheduler_start(void *(*init)(void *)) {

  init_mpu();

  sos_sched_table[0].init = init;
  sos_sched_table[0].attr.stackaddr = &_data;
  sos_sched_table[0].attr.stacksize = sos_config.sys.memory_size;
  task_init(
    CONFIG_SCHED_RR_DURATION,
    scheduler, // run the scheduler
    // Let the task init function figure out where the stack needs to be
    NULL, sos_config.sys.memory_size);

  sos_handle_event(SOS_EVENT_ROOT_FATAL, "scheduler failed");
}

void scheduler_prepare() {
  scheduler_timing_init();
  cortexm_set_unprivileged_mode(); // Enter unpriv mode
}

/*! @} */
