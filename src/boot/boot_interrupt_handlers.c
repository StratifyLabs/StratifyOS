
#include "cortexm/cortexm.h"
#include "cortexm/fault.h"

// prevent linkage to real handlers
void cortexm_hardfault_handler() {}
void cortexm_memfault_handler() {}
void cortexm_busfault_handler() {}
void cortexm_usagefault_handler() {}
void cortexm_systick_handler() {}
void cortexm_svcall_handler() {}
void cortexm_pendsv_handler() {}
void cortexm_nmi_handler() {}
void cortexm_debug_monitor_handler() {}

// used by startup for newlib
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  return 0;
}
