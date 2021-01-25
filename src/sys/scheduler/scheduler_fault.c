// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <string.h>

#include "scheduler_local.h"
#include "sos/sos.h"

#include "cortexm/fault_local.h"
#include "cortexm/util.h"

int scheduler_root_fault_handler(void *context, const mcu_event_t *data) {

  if (data != NULL) {

#if SOS_DEBUG
    char buffer[128];
    scheduler_fault_build_string(buffer, "\n");
    sos_config.debug.write(buffer, strnlen(buffer, 128));
#endif

    if (sos_config.debug.trace_event != NULL) {
      link_trace_event_t event;
      event.posix_trace_event.posix_event_id = 0;
      event.posix_trace_event.posix_pid = task_get_pid(task_get_current());
      event.posix_trace_event.posix_thread_id = task_get_current();
      event.posix_trace_event.posix_timestamp_tv_sec = 0;
      event.posix_trace_event.posix_timestamp_tv_nsec = 0;
      scheduler_fault_build_trace_string((char *)event.posix_trace_event.data);
      sos_config.debug.trace_event(&event);
    }

#if SOS_DEBUG
    sos_handle_event(SOS_EVENT_ROOT_FATAL, buffer);
#else
    sos_handle_event(SOS_EVENT_ROOT_FATAL, "OS FAULT");
#endif
  } else {
    const u32 psp;
    cortexm_get_thread_stack_ptr((void **)&psp);
    const u32 top_of_stack = (u32)sos_task_table[task_get_current()].mem.data.address
                             + sos_task_table[task_get_current()].mem.data.size;

    // check the PSP for the LR value
#if 0
    sos_trace_stack_with_pointer(
      (const u32 *)m_cortexm_fault.fault.caller, (const u32 *)psp,
      (const u32 *)top_of_stack, 32);
#endif

    sos_debug_printf(
      "stack %p + %ld\n", sos_task_table[task_get_current()].mem.data.address,
      sos_task_table[task_get_current()].mem.data.size);

#if defined SOS_DEBUG
    char buffer[128];
    scheduler_fault_build_string(buffer, 0);
    sos_debug_log_error(SOS_DEBUG_SYS, "Task Fault:%d:%s", task_get_current(), buffer);
    // check for a stack overflow error
    if (psp <= top_of_stack) {
      sos_debug_log_error(SOS_DEBUG_SYS, "Stack Overflow");
    }
#endif
    sos_handle_event(SOS_EVENT_ROOT_APPLICATION_FAULT, NULL);
  }

  scheduler_root_update_on_sleep();
  return 0;
}

void scheduler_fault_build_memory_string(char *dest, const char *term) {
  char hex_buffer[9];
  strcpy(dest, "STACK:B");
  htoa(hex_buffer, (u32)sos_task_table[task_get_current()].sp);
  strcat(dest, hex_buffer);

  strcat(dest, ":C");
  htoa(hex_buffer, (u32)sos_sched_table[task_get_current()].attr.stackaddr);
  strcat(dest, hex_buffer);

  strcat(dest, ":S");
  htoa(hex_buffer, (u32)sos_sched_table[task_get_current()].attr.stacksize);
  strcat(dest, hex_buffer);
  if (term) {
    strcat(dest, term);
  }
}

void scheduler_fault_build_trace_string(char *dest) {
  char hex_buffer[9];
  strcpy(dest, "F");
  htoa(hex_buffer, m_cortexm_fault.fault.num);
  strcat(dest, hex_buffer);
  strcat(dest, ":");
  htoa(hex_buffer, (u32)m_cortexm_fault.fault.addr);
  strcat(dest, hex_buffer);
}

void scheduler_fault_build_string(char *dest, const char *term) {
  char hex_buffer[9];
  strcpy(dest, "I");
  htoa(hex_buffer, m_cortexm_fault.tid);
  strcat(dest, hex_buffer);

  strcat(dest, ":F");
  htoa(hex_buffer, m_cortexm_fault.fault.num);
  strcat(dest, hex_buffer);

  strcat(dest, ":A");
  htoa(hex_buffer, (u32)m_cortexm_fault.fault.addr);
  strcat(dest, hex_buffer);

  strcat(dest, ":PC");
  htoa(hex_buffer, (u32)m_cortexm_fault.fault.pc);
  strcat(dest, hex_buffer);

  strcat(dest, ":C");
  htoa(hex_buffer, (u32)m_cortexm_fault.fault.caller);
  strcat(dest, hex_buffer);

  strcat(dest, ":HA");
  htoa(hex_buffer, (u32)m_cortexm_fault.fault.handler_pc);
  strcat(dest, hex_buffer);

  strcat(dest, ":HC");
  htoa(hex_buffer, (u32)m_cortexm_fault.fault.handler_caller);
  strcat(dest, hex_buffer);

  if (term) {
    strcat(dest, term);
  }
}
