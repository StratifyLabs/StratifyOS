// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

//#include "config.h"

#include <errno.h>
#include <signal.h>

#include "cortexm/cortexm.h"
#include "cortexm/mpu.h"

#include "sos/sos.h"

#include "fault_local.h"
#include "task_local.h"

#include "sos/debug.h"

volatile cortexm_fault_t m_cortexm_fault MCU_SYS_MEM;
mcu_event_handler_t m_cortexm_fault_handler MCU_SYS_MEM;

extern void cortexm_pendsv_handler();

extern void sos_root_trace_event(void *info);

void cortexm_fault_event_handler(fault_t *fault) {
  int pid;

  pid = task_get_pid(task_get_current());

  if (m_cortexm_fault.fault.num == 0) {
    m_cortexm_fault.tid = task_get_current();
    m_cortexm_fault.pid = pid;
    m_cortexm_fault.fault = *fault;
    // memcpy((void *)&m_cortexm_fault.fault, fault, sizeof(fault_t));

    // grab stack and memory usage
    u32 tid = m_cortexm_fault.tid;
    u32 tid_thread_zero = task_get_thread_zero(pid);
    u32 end_of_heap = task_calculate_heap_end(tid_thread_zero);

    volatile void *stack;

    // free heap size is thread 0 stack location - end of the heap

    if (tid == tid_thread_zero) {
      cortexm_get_thread_stack_ptr((void **)&stack);
    } else {
      stack = sos_task_table[tid_thread_zero].sp;
    }

    m_cortexm_fault.free_heap_size = (u32)stack - end_of_heap;

    if (task_thread_asserted(tid)) {
      // since this is a thread the stack is on the heap (malloc'd)
      cortexm_get_thread_stack_ptr((void **)&stack);
      m_cortexm_fault.free_stack_size = (u32)stack
                                        - (u32)sos_sched_table[tid].attr.stackaddr
                                        - SCHED_DEFAULT_STACKGUARD_SIZE;
    } else {
      // free stack is the same as the free heap for first thread in process
      m_cortexm_fault.free_stack_size = m_cortexm_fault.free_heap_size;
    }
  }

  if ((pid == 0) || (task_enabled_active_not_stopped(task_get_current()) == 0)) {

    m_cortexm_fault_handler.callback(m_cortexm_fault_handler.context, (void *)1);

  } else {

    // send a signal to kill the task
    for (int i = 1; i < task_get_total(); i++) {
      if (task_get_pid(i) == pid) {
        // stop running the task
        task_root_delete(i);
      }
    }

    m_cortexm_fault_handler.callback(m_cortexm_fault_handler.context, NULL);

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
