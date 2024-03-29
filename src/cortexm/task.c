// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <string.h>

#include "sos_config.h"

#include "cortexm/task.h"
#include "sos/sos.h"
#include "sos/debug.h"
#include "sos/symbols.h"
#include "task_local.h"

#define SYSTICK_MIN_CYCLES 10000

volatile task_t sos_task_table[CONFIG_TASK_TOTAL] MCU_SYS_MEM;

volatile s8 m_task_current_priority MCU_SYS_MEM;
static volatile u8 m_task_exec_count MCU_SYS_MEM;
int m_task_rr_reload MCU_SYS_MEM;
volatile int m_task_current MCU_SYS_MEM;
static void svcall_read_rr_timer(u32 *val) MCU_ROOT_CODE;
static int set_systick_interval(int interval) MCU_ROOT_EXEC_CODE;
static void switch_contexts() MCU_ROOT_EXEC_CODE;
static void task_check_count_flag() MCU_ROOT_EXEC_CODE;

static void system_reset(); // This is used if the OS process returns
void system_reset() { cortexm_svcall(cortexm_reset, NULL); }
u8 task_get_total() { return CONFIG_TASK_TOTAL; }
u8 task_get_exec_count() { return m_task_exec_count; }

void task_root_elevate_current_priority(s8 value) {
  cortexm_disable_interrupts();
  if (value > m_task_current_priority) {
    m_task_current_priority = value;
  }
  cortexm_enable_interrupts();
}

int task_init(
  int interval,
  void (*scheduler_function)(),
  void *system_memory,
  int system_memory_size) {
  void *system_stack;
  hw_stack_frame_t *frame;
  int i;

  // Initialize the main process
  if (system_memory == NULL) {
    system_memory = &_data;
  }

  system_stack = (u8 *)system_memory + system_memory_size;

  sos_task_table[0].sp = (u8 *)system_stack - sizeof(hw_stack_frame_t);
  sos_task_table[0].flags = TASK_FLAGS_EXEC | TASK_FLAGS_USED | TASK_FLAGS_ROOT;
  sos_task_table[0].parent = 0;
  sos_task_table[0].priority = 0;
  sos_task_table[0].pid = 0;
  sos_task_table[0].reent = _impure_ptr;
  sos_task_table[0].global_reent = _global_impure_ptr;

  frame = (hw_stack_frame_t *)sos_task_table[0].sp;
  frame->r0 = 0;
  frame->r1 = 0;
  frame->r2 = 0;
  frame->r3 = 0;
  frame->r12 = 0;
  frame->pc = ((u32)scheduler_function);
  frame->lr = (u32)system_reset;
  frame->psr = 0x21000000; // default PSR value
#if __FPU_USED != 0
  sos_task_table[0].fpscr = FPU->FPDSCR;
#endif

  // enable use of PSP
  cortexm_set_thread_stack_ptr((void *)sos_task_table[0].sp);
  m_task_current = 0;
  m_task_current_priority = 0;

  // Set the interrupt priorities
  for (i = 0; i <= sos_config.mcu.interrupt_request_total; i++) {
    sos_config.mcu.set_interrupt_priority(
      i, sos_config.mcu.interrupt_middle_priority * 2 - 1); // mark as middle priority
  }

  // must be same priority as microsecond timer
  sos_config.mcu.set_interrupt_priority(
    SysTick_IRQn, sos_config.mcu.interrupt_middle_priority * 2 - 1);
  sos_config.mcu.set_interrupt_priority(
    PendSV_IRQn, sos_config.mcu.interrupt_middle_priority * 2 - 1);
  sos_config.mcu.set_interrupt_priority(
    SVCall_IRQn, sos_config.mcu.interrupt_middle_priority * 2 - 1);

  // highest priority
  sos_config.mcu.set_interrupt_priority(DebugMonitor_IRQn, 0);
  // high but leave room for WDT
  sos_config.mcu.set_interrupt_priority(HardFault_IRQn, 2);
  sos_config.mcu.set_interrupt_priority(MemoryManagement_IRQn, 3);
  sos_config.mcu.set_interrupt_priority(BusFault_IRQn, 3);
  sos_config.mcu.set_interrupt_priority(UsageFault_IRQn, 3);

  // enable the FPU if it is in use
#if __FPU_USED != 0
  SCB->CPACR =
    (1 << 20) | (1 << 21) | (1 << 22) | (1 << 23); // allow full access to co-processor
  asm volatile("ISB");

  // FPU->FPCCR = (1<<31) | (1<<30); //set CONTROL<2> when FPU is used, enable lazy state
  // preservation
  FPU->FPCCR = 0; // don't automatically save the FPU registers -- save them manually
#endif

  // Turn on the task timer (MCU implementation dependent)
  cortexm_fault_init();
  set_systick_interval(interval);
  sos_task_table[0].rr_time = m_task_rr_reload;
  cortexm_set_stack_ptr((void *)&_top_of_stack); // reset the handler stack pointer
  cortexm_enable_systick_irq();                  // Enable context switching
  cortexm_set_vector_table_addr(sos_config.sys.vector_table);

  sos_config.cache.enable();

  task_root_switch_context();
  cortexm_enable_interrupts(); // Enable the interrupts

  sos_handle_event(SOS_EVENT_ROOT_FATAL, "task_init");
  while (1) {
  }

  return 0;
}

int set_systick_interval(int interval) {
  u32 reload;
  int core_tick_freq;
  reload = (sos_config.sys.core_clock_frequency * interval + 500) / 1000;
  if (reload > (0x00FFFFFF)) {
    reload = (0x00FFFFFF);
  } else if (reload < SYSTICK_MIN_CYCLES) {
    reload = SYSTICK_MIN_CYCLES;
  }
  core_tick_freq = sos_config.sys.core_clock_frequency / reload;
  cortexm_set_systick_reload(reload);
  m_task_rr_reload = reload;
  cortexm_start_systick();
  SCB->CCR = 0;
  return core_tick_freq;
}

int task_create_thread(
  void *(*p)(void *),
  void (*cleanup)(void *),
  void *arg,
  void *mem_addr,
  int mem_size,
  int pid) {
  int thread_zero;
  void *stackaddr;
  new_task_t task;

  // valid validity of pid and stack
  thread_zero = task_get_thread_zero(pid);
  stackaddr = (u8 *)mem_addr + mem_size;
  if ((thread_zero < 0) || ((u32)stackaddr & 0x03)) {
    // pid doesn't exist or stackaddr is misaligned
    return 0;
  }

  // Initialize the task
  task.stackaddr = stackaddr; // security?
  task.start = (u32)p;
  task.stop = (u32)cleanup;
  task.r0 = (u32)arg;
  task.r1 = 0;
  task.pid = pid; // security?
  task.flags = TASK_FLAGS_USED | TASK_FLAGS_THREAD;
  task.parent = task_get_parent(thread_zero); // security?
  task.priority = 0;                          // security?
  task.reent = mem_addr;                      // security?
  task.global_reent = sos_task_table[thread_zero].global_reent;
  task.mem = (void *)&(sos_task_table[thread_zero].mem);

  // Do a priv call while accessing the task table so there are no interruptions
  cortexm_svcall((cortexm_svcall_t)task_svcall_new_task, &task);
  return task.tid;
}

void task_svcall_new_task(new_task_t *task) {
  CORTEXM_SVCALL_ENTER();
  int i;

  // validate arguments
  for (i = 1; i < task_get_total(); i++) {
    if (!task_used_asserted(i)) {
      // initialize the process stack pointer
      sos_task_table[i].pid = task->pid;
      sos_task_table[i].parent = task->parent;
      sos_task_table[i].flags = task->flags;
      // never start a task with root set -- call seteuid() to make root
      sos_task_table[i].flags &= ~TASK_FLAGS_ROOT;
      sos_task_table[i].sp =
        (u8 *)task->stackaddr - sizeof(hw_stack_frame_t) - sizeof(sw_stack_frame_t);
      sos_task_table[i].reent = task->reent;
      sos_task_table[i].global_reent = task->global_reent;
      sos_task_table[i].timer.t = 0;
      sos_task_table[i].rr_time = m_task_rr_reload;
      memcpy((void *)&(sos_task_table[i].mem), task->mem, sizeof(task_memories_t));
#if __FPU_USED != 0
      sos_task_table[i].fpscr = FPU->FPDSCR;
      memset((void *)sos_task_table[i].fp, 0, sizeof(sos_task_table[i].fp));
#endif
      break;
    }
  }

  if (i == task_get_total()) {
    task->tid = 0;
  } else {
    // Initialize the stack frame
    hw_stack_frame_t *frame;
    frame = (hw_stack_frame_t *)((u8 *)task->stackaddr - sizeof(hw_stack_frame_t));
    frame->r0 = task->r0;
    frame->r1 = task->r1;
    frame->r2 = 0;
    frame->r3 = 0;
    frame->r12 = 0;
    frame->pc = task->start;
    frame->lr = task->stop;
    frame->psr = 0x21000000; // default PSR value

    task->tid = i;
  }
}

void task_root_delete(int id) {
  if ((id < task_get_total()) && (id >= 1)) {
    task_deassert_used(id);
    task_deassert_exec(id);
  }
}

void task_root_resetstack(int id) {
  // set the stack pointer to the original value
  sos_task_table[id].sp =
    (void
       *)((u32)sos_task_table[id].mem.data.address + sos_task_table[id].mem.data.size - (sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t)));
  if (id == task_get_current()) { // make effective now
    cortexm_set_thread_stack_ptr((void *)sos_task_table[id].sp);
  }
}

void *task_get_sbrk_stack_ptr(struct _reent *reent_ptr) {
  int i;
  for (i = 0; i < task_get_total(); i++) {

    // If the reent and global reent are the same then this is the main thread
    if (
      (sos_task_table[i].reent == reent_ptr)
      && (sos_task_table[i].global_reent == reent_ptr)) {

      // If the main thread is not in use, the stack is not valid
      if (task_used_asserted(i)) {
        if (i == task_get_current()) {
          void *stackaddr;
          // If the main thread is the current thread return the current stack
          // security? can set stackaddr to any value -- only write valid locations
          cortexm_svcall(cortexm_svcall_get_thread_stack_ptr, &stackaddr);
          return stackaddr;
        } else {
          // Return the stack value from thread 0 if another thread is running
          return (void *)sos_task_table[i].sp;
        }
      } else {
        // The main thread is not in use, so there is no valid stack value
        return NULL;
      }
    }
  }

  // No task table (or not matching reent structure) so no valid stack value
  return NULL;
}

int task_get_thread_zero(int pid) {
  int i;
  for (i = 0; i < task_get_total(); i++) {
    if (task_used_asserted(i)) {
      if (pid == task_get_pid(i) && !task_thread_asserted(i)) {
        return i;
      }
    }
  }
  return -1;
}

static void svcall_read_rr_timer(u32 *val) {
  CORTEXM_SVCALL_ENTER();
  *val = m_task_rr_reload - SysTick->VAL; // cppcheck-suppress[ConfigurationNotChecked]
}

u64 task_root_gettime(int tid) {
  u32 val;
  if (tid != task_get_current()) {
    return sos_task_table[tid].timer.t + (m_task_rr_reload - sos_task_table[tid].rr_time);
  } else {
    svcall_read_rr_timer(&val);
    return sos_task_table[tid].timer.t + val;
  }
}

u64 task_gettime(int tid) {
  u32 val;
  if (tid != task_get_current()) {
    return sos_task_table[tid].timer.t + (m_task_rr_reload - sos_task_table[tid].rr_time);
  } else {
    // security? args is written
    cortexm_svcall((cortexm_svcall_t)svcall_read_rr_timer, &val);
    return sos_task_table[tid].timer.t + val;
  }
}



void switch_contexts() {
  // Save the PSP to the current task's stack pointer
  SOS_DEBUG_ENTER_CYCLE_SCOPE_AVERAGE();
  asm volatile("MRS %0, psp\n\t" : "=r"(sos_task_table[m_task_current].sp));

  if (SCB->SHCSR & (1 << 15)) {
    /*
     * This means the SVCall instruction happened at
     * the same time as the context switching exception.
     *
     * But the SVCall handler was not executed because it
     * has a lower priority.
     *
     * Save the status of the SVCall interrupt so that
     * the SVCall doesn't execute in the wrong context.
     *
     *
     */
    task_assert_yield(task_get_current());
    SCB->SHCSR &= ~(1 << 15);
  }

#if __FPU_USED == 1
  volatile void *fpu_stack;
  if (m_task_current != 0) {
    // only do this if the task has used the FPU -- copy FPU registers to task table
    fpu_stack = sos_task_table[m_task_current].fp + 32;
    asm volatile("VMRS %0, fpscr\n\t" : "=r"(sos_task_table[m_task_current].fpscr));
    asm volatile("mov r1, %0\n\t" : : "r"(fpu_stack));
    asm volatile("vstmdb r1!, {s0-s31}\n\t");
  }
#endif

  do {
    m_task_current++;
    if (m_task_current == task_get_total()) {
      // The scheduler only uses OS mem -- disable the process MPU regions
      m_task_current = 0;
      if (sos_task_table[0].rr_time < SYSTICK_MIN_CYCLES) {
        sos_task_table[0].rr_time = m_task_rr_reload;
        sos_task_table[0].timer.t += (m_task_rr_reload);
      }

      // see if all tasks have used up their RR time
      int i;
      for (i = 1; i < task_get_total(); i++) {
        if (task_exec_asserted(i) && (sos_task_table[i].rr_time >= SYSTICK_MIN_CYCLES)) {
          break;
        }
      }

      // if all executing tasks have used up their RR time -- reload the RR time for
      // executing tasks
      if (i == task_get_total()) {
        for (i = 1; i < task_get_total(); i++) {
          if (task_exec_asserted(i)) {
            sos_task_table[i].timer.t += (m_task_rr_reload - sos_task_table[i].rr_time);
            sos_task_table[i].rr_time = m_task_rr_reload;
          }
        }
      }

      break;
    } else if (task_exec_asserted(m_task_current)) {
      if (
        (sos_task_table[m_task_current].rr_time >= SYSTICK_MIN_CYCLES)
        ||                                    // is there time remaining on the RR
        task_fifo_asserted(m_task_current)) { // is this a FIFO task
        // check to see if task is low on memory -- kill if necessary?

        break;
      }
    }
  } while (1);

  // Enable the MPU for the task stack guard
#if MPU_PRESENT || __MPU_PRESENT
  MPU->RBAR = (u32)(sos_task_table[m_task_current].mem.code.rbar);
  MPU->RASR = (u32)(sos_task_table[m_task_current].mem.code.rasr);

  // Enable the MPU for the process data section
  MPU->RBAR = (u32)(sos_task_table[m_task_current].mem.data.rbar);
  MPU->RASR = (u32)(sos_task_table[m_task_current].mem.data.rasr);

  MPU->RBAR = (u32)(sos_task_table[m_task_current].mem.stackguard.rbar);
  MPU->RASR = (u32)(sos_task_table[m_task_current].mem.stackguard.rasr);

  // disable MPU for ROOT tasks
  if (task_root_asserted(m_task_current)) {
    mpu_disable();
  } else {
    mpu_enable();
  }
  SOS_DEBUG_EXIT_CYCLE_SCOPE_AVERAGE(SOS_DEBUG_TASK, switch_contexts, 5000);

#endif

  _impure_ptr = sos_task_table[m_task_current].reent;
  _global_impure_ptr = sos_task_table[m_task_current].global_reent;

  if (task_fifo_asserted(m_task_current)) {
    // disable the systick interrupt (because this is a fifo task)
    cortexm_disable_systick_irq();
  } else {
    // init sys tick to the amount of time remaining
    SysTick->LOAD = sos_task_table[m_task_current]
                      .rr_time; // cppcheck-suppress[ConfigurationNotChecked]
    SysTick->VAL = 0; // cppcheck-suppress[ConfigurationNotChecked] force a reload
    // enable the systick interrupt
    cortexm_enable_systick_irq();
  }

#if __FPU_USED == 1
  // only do this if the task has used the FPU
  // task_load_fpu();
  if (m_task_current != 0) {
    fpu_stack = sos_task_table[m_task_current].fp;
    asm volatile("VMSR fpscr, %0\n\t" : : "r"(sos_task_table[m_task_current].fpscr));
    asm volatile("mov r1, %0\n\t" : : "r"(fpu_stack));
    asm volatile("vldm r1!, {s0-s31}\n\t");
  }
#endif

  if (task_yield_asserted(task_get_current())) {
    // this means the SYSTICK excepted at the same time as the SVC instruction
    SCB->SHCSR |= (1 << 15);
    task_deassert_yield(task_get_current());
  }

  // write the new task's stack pointer to the PSP
  asm volatile("MSR psp, %0\n\t" : : "r"(sos_task_table[m_task_current].sp));
}

void task_root_switch_context() {

  // cppcheck-suppress[ConfigurationNotChecked] save the RR time from the SYSTICK
  sos_task_table[task_get_current()].rr_time = SysTick->VAL;

  // set the pend SV interrupt pending -- causes cortexm_pendsv_handler() to execute when
  // current interrupt exits
  SCB->ICSR |= (1 << 28);
}

void task_check_count_flag() {
  // check the countflag
  if (SysTick->CTRL & (1 << 16)) { // cppcheck-suppress[ConfigurationNotChecked]
    sos_task_table[m_task_current].rr_time = 0;
    switch_contexts();
  }
}

void cortexm_systick_handler() MCU_WEAK;
void cortexm_systick_handler() {
  task_save_context();
  task_check_count_flag();
  task_load_context();
  task_return_context();
}

// Weak is needed for overriding when building bootloader
void cortexm_pendsv_handler() MCU_WEAK;
void cortexm_pendsv_handler() {
  task_save_context();

  // disable interrupts -- Re-entrant scheduler issue #130
  m_task_exec_count = 0;
  SOS_DEBUG_ENTER_CYCLE_SCOPE_AVERAGE();
  cortexm_disable_interrupts();
  int i;
  for (i = 1; i < task_get_total(); i++) {

    if (
      task_enabled_active_not_stopped(i) && // enabled, active, and not stopped
      (task_get_priority(i)
       == task_get_current_priority())) { // task is equal to the currently executing
                                          // priority
      // Enable process execution for highest active priority tasks
      task_assert_exec(i);
      m_task_exec_count++;
    } else {
      // Disable process execution for lower priority tasks
      task_deassert_exec(i);
    }
  }

  // enable interrupts -- Re-entrant scheduler issue
  cortexm_enable_interrupts();
  SOS_DEBUG_EXIT_CYCLE_SCOPE_AVERAGE(SOS_DEBUG_TASK, pend_critical, 1000);

  // switch contexts if current task is not executing or it wants to yield
  if (
    (task_get_current()) == 0 || // always switch away from task zero if requested
    (task_exec_asserted(task_get_current()) == 0)
    || // checks if current task is NOT running
    task_yield_asserted(
      task_get_current())) { // checks if current task requested a context switch
    task_deassert_yield(task_get_current());
    switch_contexts();
  }

  task_load_context();
  task_return_context();
}

static void svcall_restore_task(void *args) MCU_NAKED;
void svcall_restore_task(void *args) {
  asm volatile("push {lr}\n\t");
  u32 pstack;
  CORTEXM_SVCALL_ENTER();

  // discard the current HW stack by adjusting the PSP up by sizeof(hw_stack_frame_t)
  // --sw_stack_frame_t is same size
  pstack = __get_PSP();

  __set_PSP(pstack + sizeof(hw_stack_frame_t));

  // Load the software context that is on the stack from the pre-interrupted task
  task_load_context();
  // This function will now return to the original execution stack
  asm volatile("pop {pc}\n\t");
}

void task_restore() {
  // handlers inserted with task_interrupt() must call this function when the task
  // completes in order to restore the stack
  cortexm_svcall(svcall_restore_task, NULL);
}

int task_root_interrupt_call(void *args) {
  u32 pstack;
  task_interrupt_t *intr = (task_interrupt_t *)args;

  if (task_enabled(intr->tid)) {
    hw_stack_frame_t *hw_frame;
    if (intr->tid == task_get_current()) {
      pstack = __get_PSP();
      __set_PSP(pstack - sizeof(hw_stack_frame_t));
      hw_frame = (hw_stack_frame_t *)__get_PSP(); // frame now points to the new HW stack

      // current tid is interrupting tid so software stack is not changed

    } else {
      // Since this is another task, the current PSP is not touched
      hw_frame =
        (hw_stack_frame_t *)(sos_task_table[intr->tid].sp - sizeof(hw_stack_frame_t));
      sos_task_table[intr->tid].sp =
        sos_task_table[intr->tid].sp
        - (sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t));
    }

    hw_frame->r0 = intr->arg[0];
    hw_frame->r1 = intr->arg[1];
    hw_frame->r2 = intr->arg[2];
    hw_frame->r3 = intr->arg[3];
    hw_frame->r12 = 0;
    hw_frame->pc = (u32)intr->handler;
    hw_frame->lr = (u32)task_restore;
    hw_frame->psr = 0x21000000; // default PSR value
  }

  if (intr->sync_callback != NULL) {
    intr->sync_callback(intr->sync_callback_arg);
  }

  if (intr->tid != task_get_current()) {
    // this task is not the currently executing task
    return 1;
  }
  return 0;
}

u32 task_interrupt_stacksize() {
  return sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t);
}

void task_svcall_interrupt(void *args) {
  task_save_context(); // save the current software context
  // This function inserts the handler on the specified task's stack
  CORTEXM_SVCALL_ENTER();

  if (task_root_interrupt_call(args)) {
    task_load_context(); // the context needs to be loaded for the current task --
                         // otherwise it is loaded by the switcher
  }
}

int task_interrupt(task_interrupt_t *intr) {
  if (intr->tid < task_get_total()) {
    cortexm_svcall(task_svcall_interrupt, intr);
    return 0;
  }
  return -1;
}

u32 task_calculate_heap_end(u32 task_id) {
  if (
    (task_id < task_get_total()) && (task_thread_asserted(task_id) == 0)
    && (sos_task_table[task_id].reent != NULL)) {

    return (u32)
           & (((struct _reent *)sos_task_table[task_id].reent)->procmem_base->base)
               + ((struct _reent *)sos_task_table[task_id].reent)->procmem_base->size;
  }
  return 0;
}
