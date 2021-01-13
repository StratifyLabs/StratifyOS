// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/mpu.h"
#include "cortexm_local.h"
#include "mcu/core.h"
#include "mcu/mcu.h"
#include "sos/sos.h"

extern int sos_main();

// this is used to ensure svcall's execute from start to finish
cortexm_svcall_t cortexm_svcall_validation MCU_SYS_MEM;

void cortexm_delay_systick(u32 ticks) {
  u32 countdown = ticks;
  u32 start = cortexm_get_systick_value();
  u32 value;
  u32 end;

  if (SysTick->CTRL & 0x01) { // cppcheck-suppress[ConfigurationNotChecked]
    if (countdown > cortexm_get_systick_reload() / 2) {
      countdown = cortexm_get_systick_reload() / 2;
    }

    if (countdown > start) {
      end = cortexm_get_systick_reload() - countdown + start;
      do {
        value = cortexm_get_systick_value();
      } while ((value < countdown) || (value > end));
    } else {
      end = start - countdown;
      do {
        value = cortexm_get_systick_value();
      } while ((value > end) && (value < start));
    }
  } else {
    volatile u32 i = 0;
    while (i < ticks) {
      i++;
    }
  }
}

void cortexm_delay_us(u32 us) {
  // ticks is the number of ticks in one microsecond
  u32 ticks = sos_config.clock.frequency / 1000000UL;
  cortexm_delay_systick(ticks * us);
}

void cortexm_delay_ms(u32 ms) {
  int i;
  for (i = 0; i < ms; i++) {
    cortexm_delay_us(1000);
  }
}

void cortexm_assign_zero_sum32(void *data, int count) {
  u32 sum = 0;
  u32 *ptr = data;
  int i;
  for (i = 0; i < count - 1; i++) {
    sum += ptr[i];
  }
  ptr[i] = (u32)(0 - sum);
}

void cortexm_assign_zero_sum8(void *data, int count) {
  u8 sum = 0;
  u8 *ptr = data;
  int i;
  for (i = 0; i < count - 1; i++) {
    sum += ptr[i];
  }
  ptr[i] = (u8)(0 - sum);
}

int cortexm_verify_zero_sum32(void *data, int count) {
  u32 sum = 0;
  u32 *ptr = data;
  int i;
  for (i = 0; i < count; i++) {
    sum += ptr[i];
  }
  return sum == 0;
}

int cortexm_verify_zero_sum8(void *data, int count) {
  u8 sum = 0;
  u8 *ptr = data;
  int i;
  for (i = 0; i < count; i++) {
    sum += ptr[i];
  }
  return sum == 0;
}

void cortexm_reset_mode() {
  cortexm_disable_interrupts();
  mpu_disable();
  cortexm_disable_systick_irq();
  for (s16 i = -14; i < 255; i++) {
    cortexm_disable_irq(i);
    mcu_core_set_nvic_priority(i, 0);
  }
}

void cortexm_set_privileged_mode() {
  register u32 control;
  control = __get_CONTROL();
  control &= ~0x01;
  __set_CONTROL(control);
}

void cortexm_set_unprivileged_mode() {
  register u32 control;
  control = __get_CONTROL();
  control |= 0x01;
  __set_CONTROL(control);
}

void cortexm_set_thread_mode() {
  register u32 control;
  control = __get_CONTROL();
  control |= 0x02;
  __set_CONTROL(control);
}

void cortexm_reset_handler() {
  cortexm_initialize_heap();
  sos_handle_event(SOS_EVENT_ROOT_RESET, 0);
  sos_main(); // This function should never return
  sos_handle_event(SOS_EVENT_ROOT_FATAL, "main");
  while (1) {
    ;
  }
}

void cortexm_nmi_handler() { sos_handle_event(SOS_EVENT_ROOT_FATAL, "nmi"); }

void cortexm_debug_monitor_handler() {}

void cortexm_initialize_heap() {
  u32 *src, *dest;
  src = &_etext; // point src to copy of data that is stored in flash
  for (dest = &_data; dest < &_edata;) {
    *dest++ = *src++;
  } // Copy from flash to RAM (data)
  for (src = &_bss; src < &_ebss;)
    *src++ = 0; // Zero out BSS section
  for (src = &_sys; src < &_esys;)
    *src++ = 0; // Zero out sysmem

  // Re-entrancy initialization
  // If the program faults on the next line, make sure the etext and data are
  // aligned properly in the linker script (4 byte boundary)
  _REENT->procmem_base = (proc_mem_t *)&_ebss;
  _REENT->procmem_base->size = 0;
  _REENT->procmem_base->sigactions = NULL;
  _REENT->procmem_base->siginfos = NULL;
  _REENT->procmem_base->proc_name = "sys";
  const open_file_t init_open_file = {0};
  for (int i = 0; i < OPEN_MAX; i++) {
    _REENT->procmem_base->open_file[i] = init_open_file;
  }

  // Initialize the global mutexes
  __lock_init_recursive_global(__malloc_lock_object);
  _REENT->procmem_base->__malloc_lock_object.flags |=
    PTHREAD_MUTEX_FLAGS_PSHARED; // Make the malloc lock pshared

  __lock_init_global(__tz_lock_object);
  __lock_init_recursive_global(__atexit_lock);
  __lock_init_recursive_global(__sfp_lock);
  __lock_init_recursive_global(__sinit_lock);
  __lock_init_recursive_global(__env_lock_object);
}

void cortexm_svcall(cortexm_svcall_t call, void *args) { asm volatile("SVC 0\n"); }

void cortexm_svcall_handler() {
  register u32 *frame;
  register cortexm_svcall_t call;
  register void *args;
  asm volatile("MRS %0, psp\n\t" : "=r"(frame));
  call = (cortexm_svcall_t)frame[0];
  args = (void *)(frame[1]);
  // verify call is located secure kernel region ROOT_EXEC ONLY
  if (
    (((u32)call >= (u32)&_text)
     && ((u32)call < (u32)&_etext)) // cppcheck-suppress[clarifyCondition]
    || (((u32)call >= (u32)&_tcim) && ((u32)call < (u32)&_etcim)) // cppcheck-suppress[clarifyCondition]
  ) {
    // args must point to kernel RAM or kernel flash -- can't be SYS MEM or registers or
    // anything like that
    call(args);
  } else {
    // this needs to be a fault
  }
#if 0
	//add this when security update is ready
	if( cortexm_svcall_validation != call ){
		//this is a security violation
	}
	cortexm_svcall_validation = 0;
#endif
}

int cortexm_validate_callback(mcu_callback_t callback) {
  // \todo callbacks need to be in ROOT_EXEC_ONLY
  if (
    (((u32)callback >= (u32)&_text)
     && ((u32)callback < (u32)&_etext)) // cppcheck-suppress[clarifyCondition]
    || (((u32)callback >= (u32)&_tcim) && ((u32)callback < (u32)&_etcim)) // cppcheck-suppress[clarifyCondition]
  ) {
    return 0;
  }
  return -1;
}

void cortexm_reset(void *args) { NVIC_SystemReset(); }

void cortexm_disable_irq(s16 x) { NVIC_DisableIRQ(x); }

void cortexm_enable_irq(s16 x) { NVIC_EnableIRQ(x); }

void cortexm_disable_interrupts() { asm volatile("cpsid i"); }

void cortexm_enable_interrupts() { asm volatile("cpsie i"); }

void cortexm_get_stack_ptr(void **ptr) {
  void *result = NULL;
  asm volatile("MRS %0, msp\n\t" : "=r"(result));
  *ptr = result;
}

void cortexm_set_stack_ptr(void *ptr) { asm volatile("MSR msp, %0\n\t" : : "r"(ptr)); }

void cortexm_svcall_get_thread_stack_ptr(void *ptr) {
  CORTEXM_SVCALL_ENTER();
  cortexm_get_thread_stack_ptr(ptr);
}

void cortexm_get_thread_stack_ptr(void **ptr) {
  void *result = NULL;
  asm volatile("MRS %0, psp\n\t" : "=r"(result));
  *ptr = result;
}

void cortexm_set_thread_stack_ptr(void *ptr) {
  asm volatile("MSR psp, %0\n\t" : : "r"(ptr));
}

int cortexm_set_irq_priority(int irq, int prio, u32 o_events) {

  if (o_events & MCU_EVENT_FLAG_SET_PRIORITY) {

    // calculate the relative priority (lower value is higher priority)
    prio = mcu_config.irq_middle_prio * 2 - 1 - prio;

    // zero priority is reserved for exceptions -- lower value is higher priority
    if (prio < 4) {
      prio = 4;
    }

    // ensure lowest priority (highest value) is not exceeded
    if (prio > (mcu_config.irq_middle_prio * 2 - 1)) {
      prio = mcu_config.irq_middle_prio * 2 - 1;
    }

    // now set the priority in the NVIC
    mcu_core_set_nvic_priority(irq, prio);
  }

  return 0;
}

void cortexm_set_vector_table_addr(void *addr) {
  SCB->VTOR = (uint32_t)addr;
  __DSB();
}

u32 cortexm_get_vector_table_addr() {
  __DSB();
  return SCB->VTOR;
}

u32 cortexm_get_systick_value() {
  return SysTick->VAL; // cppcheck-suppress[ConfigurationNotChecked]
}

u32 cortexm_get_systick_reload() {
  return SysTick->LOAD; // cppcheck-suppress[ConfigurationNotChecked]
}

void cortexm_set_systick_reload(u32 value) {
  SysTick->LOAD = value; // cppcheck-suppress[ConfigurationNotChecked]
}

void cortexm_start_systick() {
  SysTick->CTRL = (1 << 0) | (1 << 2); // cppcheck-suppress[ConfigurationNotChecked]
                                       // Internal Clock CPU and enable the timer
}
