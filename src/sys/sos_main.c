// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "config.h"
#include "cortexm/cortexm.h"
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

u64 sos_realtime() {
  struct mcu_timeval tv;
  if (cortexm_is_root_mode()) {
    scheduler_timing_svcall_get_realtime(&tv);
  } else {
    cortexm_svcall(scheduler_timing_svcall_get_realtime, &tv);
  }
  return scheduler_timing_real64usec(&tv);
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

bootloader_api_t *sos_get_bootloader_api() {
  if (sos_config.sys.bootloader_start_address == 0xffffffff) {
    return (bootloader_api_t
              *)(sos_config.sys.bootloader_start_address + BOOTLOADER_API_OFFSET);
  }
  return NULL;
}
