// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <sdk/api.h>

#include "cortexm/cortexm.h"

#include "check_config.h"

#include "scheduler/scheduler_timing.h"

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

    sos_debug_log_directive(
      SOS_DEBUG_MALLOC | SOS_DEBUG_TASK | SOS_DEBUG_SCHEDULER, "reset");
    sos_debug_log_directive(
      SOS_DEBUG_MALLOC, "hist:Malloc Perf:_malloc_r_us:malloc() execution time in us");
    sos_debug_log_directive(
      SOS_DEBUG_MALLOC, "hist:Free Perf:_free_r_us:free() execution time in us");
    sos_debug_log_directive(
      SOS_DEBUG_PTHREAD,
      "hist:Mutex Lock Perf:pthread_mutex_lock_us:pthread_mutex_lock() execution time in us");
    sos_debug_log_directive(
      SOS_DEBUG_PTHREAD,
      "hist:Mutex Unlock Perf:pthread_mutex_unlock_us:pthread_mutex_unlock() execution time in us");
    sos_debug_log_directive(
      SOS_DEBUG_PTHREAD,
      "hist:Condition Perf:pthread_cond_timedwait_us:pthread_cond_timedwait() execution time in us");
    sos_debug_log_directive(
      SOS_DEBUG_UNISTD, "hist:usleep Oversleep:usleep_us:usleep() oversleep time in us");
    sos_debug_log_directive(
      SOS_DEBUG_UNISTD, "hist:sleep Oversleep:sleep_us:sleep() oversleep time in us");
    sos_debug_log_directive(
      SOS_DEBUG_TASK,
      "hist:Context Switch Cycles:switch_contexts:Average number of cycles for context switching");
    sos_debug_log_directive(
      SOS_DEBUG_TASK,
      "hist:PendSV Critical Cycles:pend_critical:Average number of cycles interrupts are off on PendSV");
    sos_debug_log_directive(
      SOS_DEBUG_SCHEDULER,
      "hist:Scheduler Critical Cycles:scheduler_critical:Average number of cycles interrupts are off when stopping a task");
    sos_debug_log_directive(
      SOS_DEBUG_MALLOC, "heap:OS Heap:heap0:OS Heap Utilization over time");
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

void check_config() {
  CHECK_CONFIG(sys, get_serial_number);
  CHECK_CONFIG(sys, kernel_request);
  CHECK_CONFIG(sys, kernel_request_api);
  CHECK_CONFIG(sys, name);
  CHECK_CONFIG(sys, version);
  CHECK_CONFIG(sys, git_hash);
  CHECK_CONFIG(sys, id);
  CHECK_CONFIG(sys, team_id);
  CONFIG_ASSERT(
    sos_config.sys.memory_size > 8192, "assert: sos_config.sys.memory_size > 8192")
  CHECK_CONFIG(sys, initialize);

  CHECK_CONFIG(mcu, set_interrupt_priority);
  CHECK_CONFIG(mcu, reset_watchdog_timer);
  CHECK_CONFIG(mcu, set_pin_function);

  CHECK_CONFIG(fs, devfs_list);
  CHECK_CONFIG(fs, rootfs_list);
  CHECK_CONFIG(fs, stdin_dev);
  CHECK_CONFIG(fs, stdout_dev);
  CHECK_CONFIG(fs, stderr_dev);
  CHECK_CONFIG(fs, trace_dev);

  CHECK_CONFIG(sleep, idle);
  CHECK_CONFIG(sleep, hibernate);
  CHECK_CONFIG(sleep, powerdown);

  CHECK_CONFIG(debug, enable_led);
  CHECK_CONFIG(debug, disable_led);

  CHECK_CONFIG(cache, enable);
  CHECK_CONFIG(cache, disable);
  CHECK_CONFIG(cache, invalidate_instruction);
  CHECK_CONFIG(cache, invalidate_data);
  CHECK_CONFIG(cache, invalidate_data_block);
  CHECK_CONFIG(cache, clean_data);
  CHECK_CONFIG(cache, clean_data_block);

  CHECK_CONFIG(cache, external_sram_policy);
  CHECK_CONFIG(cache, sram_policy);
  CHECK_CONFIG(cache, flash_policy);
  CHECK_CONFIG(cache, peripherals_policy);
  CHECK_CONFIG(cache, lcd_policy);
  CHECK_CONFIG(cache, external_flash_policy);
  CHECK_CONFIG(cache, tightly_coupled_data_policy);
  CHECK_CONFIG(cache, tightly_coupled_instruction_policy);
  CHECK_CONFIG(cache, os_code_mpu_type);
  CHECK_CONFIG(cache, os_data_mpu_type);
  CHECK_CONFIG(cache, os_system_data_mpu_type);

  // cache policy check
  if (sos_config.usb.logical_endpoint_count > 0) {
    CONFIG_ASSERT(
      sos_config.usb.control_endpoint_max_size != 0,
      "assert: sos_config.usb.control_endpoint_max_size != 0");

    CHECK_CONFIG(usb, set_attributes);
    CHECK_CONFIG(usb, set_action);
    CHECK_CONFIG(usb, write_endpoint);
    CHECK_CONFIG(usb, read_endpoint);
  }

  CHECK_CONFIG(task, start);
  CONFIG_ASSERT(
    sos_config.task.start_stack_size > CONFIG_PTHREAD_STACK_MIN,
    "assert: sos_config.task.start_stack_size > CONFIG_PTHREAD_STACK_MIN")

  CHECK_CONFIG(clock, initialize);
  CHECK_CONFIG(clock, enable);
  CHECK_CONFIG(clock, disable);
  CHECK_CONFIG(clock, set_channel);
  CHECK_CONFIG(clock, get_channel);
  CHECK_CONFIG(clock, microseconds);
  CHECK_CONFIG(clock, nanoseconds);

}
