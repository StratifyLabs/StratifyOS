// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "config.h"
#include "cortexm/cortexm.h"
#include "mcu/mcu.h"
#include "sos/sos.h"

#include "scheduler/scheduler_local.h"

int sos_main() {

  if (sos_config.sys.initialize) {
    sos_config.sys.initialize();
  }

  if (sos_config.debug.initialize) {
    sos_config.debug.initialize();
  }

  scheduler_init();
  scheduler_start(sos_config.task.start);

  // should never reach this point
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
