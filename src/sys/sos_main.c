/* Copyright 2011-2021 Tyler Gilbert;
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

#include "config.h"
#include "cortexm/cortexm.h"
#include "mcu/mcu.h"
#include "sos/sos.h"

#include "scheduler/scheduler_local.h"

int sos_main() {

  if (sos_config.sys.initialize) {
    sos_config.sys.initialize();
  }

  cortexm_fault_init();
  cortexm_enable_interrupts(); // Enable the interrupts

  if (scheduler_init() < 0) { // Initialize the data used for the scheduler
    sos_handle_event(SOS_EVENT_ROOT_FATAL, "main sched init");
    cortexm_disable_interrupts();
    while (1) {
    }
  }

  if (scheduler_start(sos_config.task.start) < 0) {
    sos_handle_event(SOS_EVENT_ROOT_FATAL, "main sched start");
    cortexm_disable_interrupts();
    while (1) {
    }
  }

  sos_handle_event(SOS_EVENT_ROOT_FATAL, "main sched failed");
  cortexm_disable_interrupts();
  while (1) {
  }
  return 0;
}

void sos_handle_event(int event, void *args) {
  if (sos_config.event_handler != NULL) {
    sos_config.event_handler(event, args);
  }
}

int kernel_request(int request, void *data) {
  if (sos_config.sys.kernel_request) {
    return sos_config.sys.kernel_request(request, data);
  }
  errno = ENOSYS;
  return -1;
}

const void *kernel_request_api(u32 request) {
  if (sos_config.sys.kernel_request_api) {
    return sos_config.sys.kernel_request_api(request);
  }
  return NULL;
}
