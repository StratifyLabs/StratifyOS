#include "sos/symbols.h"

#include "cortexm/cortexm.h"
#include "cortexm/fault.h"

extern void boot_main();

void cortexm_reset_handler() {
  u32 *src, *dest;
  src = &_etext; // point src to copy of data that is stored in flash
  for (dest = &_data; dest < &_edata;) {
    *dest++ = *src++;
  } // Copy from flash to RAM (data)
  for (src = &_bss; src < &_ebss;)
    *src++ = 0; // Zero out BSS section
  for (src = &_sys; src < &_esys;)
    *src++ = 0; // Zero out sysmem

  boot_main(); // This function should never return
}

// prevent linkage to real handlers
void cortexm_hardfault_handler() {
  while (1) {
  }
}
void cortexm_memfault_handler() {
  while (1) {
  }
}
void cortexm_busfault_handler() {
  while (1) {
  }
}
void cortexm_usagefault_handler() {
  while (1) {
  }
}
void cortexm_systick_handler() {
  while (1) {
  }
}
void cortexm_svcall_handler() {
  while (1) {
  }
}
void cortexm_pendsv_handler() {
  while (1) {
  }
}
void cortexm_nmi_handler() {
  while (1) {
  }
}
void cortexm_debug_monitor_handler() {
  while (1) {
  }
}

// used by startup for newlib
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  return 0;
}
