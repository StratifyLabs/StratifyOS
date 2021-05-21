// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "config.h"
#include "cortexm/cortexm.h"
#include "sos/sos.h"

#include "check_config.h"

#include "scheduler/scheduler_local.h"

static void check_sys_config();
static void check_debug_config();
static void check_config();

int sos_main() {

  if (sos_config.sys.initialize) {
    sos_config.sys.initialize();
  }

  if (sos_config.debug.initialize) {
#if SOS_DEBUG
    cortexm_initialize_dwt();
#endif
    sos_config.debug.initialize();

    sos_debug_log_directive(SOS_DEBUG_MALLOC | SOS_DEBUG_TASK | SOS_DEBUG_SCHEDULER, "reset");
    sos_debug_log_directive(SOS_DEBUG_MALLOC, "hist:Malloc Perf:_malloc_r_us:malloc() execution time in us");
    sos_debug_log_directive(SOS_DEBUG_MALLOC, "hist:Free Perf:_free_r_us:free() execution time in us");
    sos_debug_log_directive(SOS_DEBUG_PTHREAD, "hist:Mutex Lock Perf:pthread_mutex_lock_us:pthread_mutex_lock() execution time in us");
    sos_debug_log_directive(SOS_DEBUG_PTHREAD, "hist:Mutex Unlock Perf:pthread_mutex_unlock_us:pthread_mutex_unlock() execution time in us");
    sos_debug_log_directive(SOS_DEBUG_PTHREAD, "hist:Condition Perf:pthread_cond_timedwait_us:pthread_cond_timedwait() execution time in us");
    sos_debug_log_directive(SOS_DEBUG_UNISTD, "hist:usleep Oversleep:usleep_us:usleep() oversleep time in us");
    sos_debug_log_directive(SOS_DEBUG_UNISTD, "hist:sleep Oversleep:sleep_us:sleep() oversleep time in us");
    sos_debug_log_directive(SOS_DEBUG_TASK, "hist:Context Switch Cycles:switch_contexts:Average number of cycles for context switching");
    sos_debug_log_directive(SOS_DEBUG_TASK, "hist:PendSV Critical Cycles:pend_critical:Average number of cycles interrupts are off on PendSV");
    sos_debug_log_directive(SOS_DEBUG_SCHEDULER, "hist:Scheduler Critical Cycles:scheduler_critical:Average number of cycles interrupts are off when stopping a task");
    sos_debug_log_directive(SOS_DEBUG_MALLOC, "heap:OS Heap:heap0:OS Heap Utilization over time");
  }


  check_config();

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

void check_sys_config() {
  // SOS_EVENT_ROOT_PANIC_SYSTEM_CONFIG
}

void check_debug_config() {
  // SOS_EVENT_ROOT_PANIC_DEBUG_CONFIG
}

void check_config() {
  CHECK_SYS_CONFIG(get_serial_number);
  CHECK_SYS_CONFIG(kernel_request);
  CHECK_SYS_CONFIG(kernel_request_api);
  CHECK_SYS_CONFIG(name);
  CHECK_SYS_CONFIG(version);
  CHECK_SYS_CONFIG(git_hash);
  CHECK_SYS_CONFIG(id);
  CHECK_SYS_CONFIG(team_id);

  CHECK_MCU_CONFIG(set_interrupt_priority);
  CHECK_MCU_CONFIG(reset_watchdog_timer);
  CHECK_MCU_CONFIG(set_pin_function);

  CHECK_SLEEP_CONFIG(idle);
  CHECK_SLEEP_CONFIG(hibernate);
  CHECK_SLEEP_CONFIG(powerdown);

  CHECK_CACHE_CONFIG(enable);
  CHECK_CACHE_CONFIG(disable);
  CHECK_CACHE_CONFIG(invalidate_instruction);
  CHECK_CACHE_CONFIG(invalidate_data);
  CHECK_CACHE_CONFIG(invalidate_data_block);
  CHECK_CACHE_CONFIG(clean_data);
  CHECK_CACHE_CONFIG(clean_data_block);

  // cache policy check
  if (sos_config.usb.logical_endpoint_count > 0) {
    CHECK_USB_CONFIG(set_attributes);
    CHECK_USB_CONFIG(set_action);
    CHECK_USB_CONFIG(write_endpoint);
    CHECK_USB_CONFIG(read_endpoint);
  }


  CHECK_TASK_CONFIG(start);

  CHECK_CLOCK_CONFIG(initialize);
  CHECK_CLOCK_CONFIG(enable);
  CHECK_CLOCK_CONFIG(disable);
  CHECK_CLOCK_CONFIG(set_channel);
  CHECK_CLOCK_CONFIG(get_channel);
  CHECK_CLOCK_CONFIG(microseconds);
  CHECK_CLOCK_CONFIG(nanoseconds);

}
