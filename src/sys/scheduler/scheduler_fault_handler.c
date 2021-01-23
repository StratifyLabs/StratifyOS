// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

//#include "config.h"

#include <errno.h>
#include <signal.h>

#include "cortexm/mpu.h"
#include "sos/sos.h"

#include "../signal/sig_local.h"
#include "../unistd/unistd_local.h"
#include "scheduler_local.h"

#include "sos/debug.h"

extern void cortexm_pendsv_handler();

extern void sos_root_trace_event(void *info);

void scheduler_fault_event_handler(fault_t *fault) {
  int pid;

  pid = task_get_pid(task_get_current());

  if (m_scheduler_fault.fault.num == 0) {
    m_scheduler_fault.tid = task_get_current();
    m_scheduler_fault.pid = pid;
    m_scheduler_fault.fault = *fault;
    // memcpy((void *)&m_scheduler_fault.fault, fault, sizeof(fault_t));

    // grab stack and memory usage
    u32 tid = m_scheduler_fault.tid;
    u32 tid_thread_zero = task_get_thread_zero(pid);
    u32 end_of_heap = scheduler_calculate_heap_end(tid_thread_zero);

    volatile void *stack;

    // free heap size is thread 0 stack location - end of the heap

    if (tid == tid_thread_zero) {
      cortexm_get_thread_stack_ptr((void **)&stack);
    } else {
      stack = sos_task_table[tid_thread_zero].sp;
    }

    m_scheduler_fault.free_heap_size = (u32)stack - end_of_heap;

    if (task_thread_asserted(tid)) {
      // since this is a thread the stack is on the heap (malloc'd)
      cortexm_get_thread_stack_ptr((void **)&stack);
      m_scheduler_fault.free_stack_size = (u32)stack
                                          - (u32)sos_sched_table[tid].attr.stackaddr
                                          - SCHED_DEFAULT_STACKGUARD_SIZE;
    } else {
      // free stack is the same as the free heap for first thread in process
      m_scheduler_fault.free_stack_size = m_scheduler_fault.free_heap_size;
    }
  }

  if ((pid == 0) || (task_enabled_active_not_stopped(task_get_current()) == 0)) {

#if SOS_DEBUG
    char buffer[128];
    scheduler_fault_build_string(buffer, "\n");
    sos_config.debug.write(buffer, strnlen(buffer, 128));
#endif

    if (sos_config.debug.trace_event != NULL) {
      link_trace_event_t event;
      event.posix_trace_event.posix_event_id = 0;
      event.posix_trace_event.posix_pid = pid;
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
    // check the PSP for the LR value
    sos_trace_stack_with_pointer(
      (const u32 *)m_scheduler_fault.fault.caller, (const u32 *)psp,
      (const u32 *)top_of_stack, 32);

    // send a signal to kill the task
    for (int i = 1; i < task_get_total(); i++) {
      if (task_get_pid(i) == pid) {
        // stop running the task
        task_root_delete(i);
      }
    }
    scheduler_root_update_on_sleep();

    /* scheduler_root_update_on_sleep() sets the PEND SV interrupt.
     *
     *
     * However, the pendsv handler won't be executed until this interrupt returns.
     * When this interrupt returns it will restore the hw frame to the PSP. If the fault
     * was caused be a bad PSP value, another fault will immediately be triggered. The hw
     * frame is restored in un-privileged mode.
     *
     * By calling cortexm_pendsv_handler() manually, the context will be changed to a
     * non-faulty thread. This means the faulting PSP will never be touched again.
     *
     * The cortexm_pendsv_handler() will save the context of the faulty PSP, but the PSP
     * was reset above so, it won't overflow to memory owned by another process
     *
     *
     */
    // this will make sure the hardware has stack space to shutdown
    // on a stack overflow error
    task_root_resetstack(task_get_current());
    cortexm_pendsv_handler();
  }
}

/*! @} */
