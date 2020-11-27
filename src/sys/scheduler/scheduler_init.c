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

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */
#include "config.h"
#include "scheduler_local.h"

#include "mcu/core.h"
#include "sos/debug.h"
#include "mcu/mcu.h"

volatile scheduler_fault_t m_scheduler_fault MCU_SYS_MEM;

/*! \details This function initializes the peripheral hardware needed
 * by the scheduler specifically the microsecond timer.
 * \return Zero on success or an error code (see \ref caoslib_err_t)
 */
int scheduler_init() {
  memset((void *)sos_task_table, 0, sizeof(task_t) * sos_config.task.task_total);
  memset(
    (void *)sos_sched_table, 0, sizeof(sched_task_t) * sos_config.task.task_total);

  // Do basic init of task 0 so that memory allocation can happen before the scheduler
  // starts
  sos_task_table[0].reent = _impure_ptr;
  sos_task_table[0].global_reent = _global_impure_ptr;

  return 0;
}

/* \details This function initializes the scheduler.  It should be
 * called after all peripherals are initialized and interrupts are on.
 * \return Zero on success or an error code
 */
int scheduler_start(void *(*init)(void *)) {
  sos_sched_table[0].init = init;
  sos_sched_table[0].attr.stackaddr = &_data;
  sos_sched_table[0].attr.stacksize = sos_config.sys.memory_size;

  task_init(
    SCHED_RR_DURATION,
    scheduler, // run the scheduler
    NULL, // Let the task init function figure out where the stack needs to be and the
          // heap size
    sos_config.sys.memory_size);

  sos_led_svcall_error(0);

  // Program never gets to this point
  return -1;
}

static void svcall_prepare(void *args) {
  sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Init MPU");
  if (task_init_mpu(&_data, sos_config.sys.memory_size) < 0) {
    sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Failed to initialize memory protection");
    sos_handle_event(SOS_EVENT_ROOT_FATAL, (void *)"tski");
  }
  sos_handle_event(SOS_EVENT_ROOT_DEBUG_INITIALIZED, 0);
}

int scheduler_prepare() {

  sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Load MCU Faults");

  if (sos_debug_init() < 0) {
    cortexm_disable_interrupts();
    sos_handle_event(SOS_EVENT_ROOT_FATAL, (void *)"dbgi");
  }
  sos_debug_log_info(SOS_DEBUG_SYS, "MCU Debug started");

  cortexm_svcall(svcall_prepare, NULL);

  sos_debug_log_info(SOS_DEBUG_SCHEDULER, "Init Timing");
#if SCHED_USECOND_TMR_SLEEP_OC > -1
  if (scheduler_timing_init()) {
    return -1;
  }
#endif

  cortexm_set_unprivileged_mode(); // Enter unpriv mode
  return 0;
}

/*! @} */
