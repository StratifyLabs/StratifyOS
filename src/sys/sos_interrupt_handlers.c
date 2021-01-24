
#include "cortexm/cortexm.h"
#include "sos/events.h"
#include "sos/sos.h"
#include "sos/symbols.h"

void cortexm_reset_handler() {
  cortexm_initialize_heap();
  sos_handle_event(SOS_EVENT_ROOT_RESET, 0);
  sos_main(); // This function should never return
  sos_handle_event(SOS_EVENT_ROOT_FATAL, "main");
  while (1) {
    ;
  }
}

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

void cortexm_nmi_handler() { sos_handle_event(SOS_EVENT_ROOT_FATAL, "nmi"); }

void cortexm_debug_monitor_handler() {}
