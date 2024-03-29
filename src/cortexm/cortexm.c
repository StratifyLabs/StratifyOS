// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/mpu.h"
#include "cortexm_local.h"
#include "sos/sos.h"
#include "sos/symbols.h"

extern int sos_main();

// this is used to ensure svcall's execute from start to finish
cortexm_svcall_t cortexm_svcall_validation MCU_SYS_MEM;

void cortexm_initialize_dwt() {
  CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL = (1 << DWT_CTRL_CYCTAP_Pos) | (0xF << DWT_CTRL_POSTINIT_Pos)
              | (0xF << DWT_CTRL_POSTPRESET_Pos) | (1 << DWT_CTRL_CYCCNTENA_Pos);
}

void cortexm_enter_cycle_scope() {
  DWT->CYCCNT = 0;
  DWT->CTRL |= 0x01; // turn on the cycle counter
}

u32 cortexm_exit_cycle_scope() { return DWT->CYCCNT; }

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
  u32 ticks = sos_config.sys.core_clock_frequency / 1000000UL;
  cortexm_delay_systick(ticks * us);
}

void cortexm_delay_ms(u32 ms) {
  u32 i;
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
    sos_config.mcu.set_interrupt_priority(i, 0);
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

  // Make the malloc lock pshared
  _REENT->procmem_base->__malloc_lock_object.flags |= PTHREAD_MUTEX_FLAGS_PSHARED;

  __lock_init_global(__tz_lock_object);
  __lock_init_recursive_global(__atexit_lock);
  __lock_init_recursive_global(__sfp_lock);
  __lock_init_recursive_global(__sinit_lock);
  __lock_init_recursive_global(__env_lock_object);
}

void cortexm_svcall(cortexm_svcall_t call, void *args) { asm volatile("SVC 0\n"); }

int cortexm_validate_callback(mcu_callback_t callback) {
  // \todo callbacks need to be in ROOT_EXEC_ONLY
  if (
    (((u32)callback >= (u32)&_text)     // cppcheck-suppress[clarifyCondition]
     && ((u32)callback < (u32)&_etext)) // cppcheck-suppress[clarifyCondition]
    || (((u32)callback >= (u32)&_tcim) && ((u32)callback < (u32)&_etcim)) // cppcheck-suppress[clarifyCondition]
  ) {
    return 0;
  }
  return -1;
}

void cortexm_reset(void *args) {
  MCU_UNUSED_ARGUMENT(args);
  NVIC_SystemReset();
}

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
    prio = sos_config.mcu.interrupt_middle_priority * 2 - 1 - prio;

    // zero priority is reserved for exceptions -- lower value is higher priority
    if (prio < 4) {
      prio = 4;
    }

    // ensure lowest priority (highest value) is not exceeded
    if (prio > (sos_config.mcu.interrupt_middle_priority * 2 - 1)) {
      prio = sos_config.mcu.interrupt_middle_priority * 2 - 1;
    }

    // now set the priority in the NVIC
    sos_config.mcu.set_interrupt_priority(irq, prio);
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

const bootloader_api_t *cortexm_get_bootloader_api() {
  if (sos_config.sys.bootloader_start_address == 0xffffffff) {
    return NULL;
  }

  const bootloader_api_t **papi =
    (const bootloader_api_t *
       *)(sos_config.sys.bootloader_start_address + BOOTLOADER_API_OFFSET);
  return *papi;
}

u32 cortexm_get_hardware_id() {
  const bootloader_api_t *api = cortexm_get_bootloader_api();
  if (api) {
    return api->hardware_id;
  }
  return 0xffffffff;
}
