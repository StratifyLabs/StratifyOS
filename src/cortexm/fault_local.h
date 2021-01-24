#ifndef FAULT_LOCAL_H
#define FAULT_LOCAL_H

#include "cortexm/fault.h"

typedef struct {
  u32 tid;
  s32 free_stack_size;
  s32 free_heap_size;
  u32 pid;
  fault_t fault;
} cortexm_fault_t;

extern volatile cortexm_fault_t m_cortexm_fault MCU_SYS_MEM;
extern mcu_event_handler_t m_cortexm_fault_handler MCU_SYS_MEM;

#endif // FAULT_LOCAL_H
