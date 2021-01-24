// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/fault.h"
#include "sos/events.h"
#include "task_local.h"

static void
hardfault_handler(u32 fault_status, hw_stack_frame_t *handler_stack) MCU_ROOT_EXEC_CODE;
static void
busfault_handler(u32 bus_status, hw_stack_frame_t *handler_stack) MCU_ROOT_EXEC_CODE;
static void
memfault_handler(u32 mem_status, hw_stack_frame_t *handler_stack) MCU_ROOT_EXEC_CODE;
static void
usagefault_handler(u32 usage_status, hw_stack_frame_t *handler_stack) MCU_ROOT_EXEC_CODE;

#define get_pc(stack_reg) (((hw_stack_frame_t *)(stack_reg))->pc)
#define get_link(stack_reg) (((hw_stack_frame_t *)(stack_reg))->lr)

int cortexm_fault_init() {
  // initialize the fault handling registers
  // Bus, mem, and usage faults are enabled so that they do not cause a hard fault
  SCB->SHCSR |=
    (SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk);

  return 0;
}

void cortexm_hardfault_handler() MCU_WEAK;
void cortexm_hardfault_handler() {
  register hw_stack_frame_t *handler_stack;
  asm volatile("MRS %0, msp\n\t" : "=r"(handler_stack));
  register u32 fault_status;

  fault_status = SCB->HFSR;
  SCB->HFSR = fault_status; // Clear the hard fault

  hardfault_handler(fault_status, handler_stack);
}

void hardfault_handler(u32 fault_status, hw_stack_frame_t *handler_stack) {
  hw_stack_frame_t *stack;
  fault_t fault;
  cortexm_get_thread_stack_ptr((void **)&stack);

  fault.addr = (void *)0xFFFFFFFF;
  fault.num = MCU_FAULT_HARD_UNKNOWN;

  if ((fault_status & (1 << 30))) {

    fault_status = SCB->CFSR;
    SCB->CFSR = fault_status;

    if (fault_status & 0xFF) {
      return memfault_handler(0, handler_stack);
    } else if (fault_status & 0xFF00) {
      return busfault_handler(fault_status >> 8, handler_stack);
    } else if (fault_status & 0xFFFF0000) {
      return usagefault_handler(fault_status >> 16, handler_stack);
    }
  }

  if (fault_status & (1 << 1)) {
    fault.num = MCU_FAULT_HARD_VECTOR_TABLE;
    fault.addr = (void *)stack->pc;
  }

  fault.pc = (void *)stack->pc;
  fault.caller = (void *)stack->lr;
  fault.handler_pc = (void *)(handler_stack)->pc;
  fault.handler_caller = (void *)(handler_stack)->lr;

  cortexm_fault_event_handler(&fault);
}

void cortexm_wdtfault_handler(void *stack) {
  fault_t fault;
  hw_stack_frame_t *handler_stack = stack;
  hw_stack_frame_t *user_stack;
  cortexm_get_thread_stack_ptr((void **)&user_stack);

  fault.num = MCU_FAULT_WDT;
  fault.addr = (void *)-1;
  fault.pc = (void *)user_stack->pc;
  fault.caller = (void *)user_stack->lr;
  fault.handler_pc = (void *)handler_stack->pc;
  fault.handler_caller = (void *)handler_stack->lr;

  sos_handle_event(SOS_EVENT_ROOT_WDT_TIMEDOUT, &fault);
}

void cortexm_memfault_handler() MCU_WEAK;
void cortexm_memfault_handler() {
  register void *handler_stack;
  asm volatile("MRS %0, msp\n\t" : "=r"(handler_stack));
  register u32 status;
  status = SCB->CFSR;
  SCB->CFSR = status;
  memfault_handler(status, handler_stack);
}

void memfault_handler(u32 mem_status, hw_stack_frame_t *handler_stack) {
  fault_t fault;
  hw_stack_frame_t *stack;
  cortexm_get_thread_stack_ptr((void **)&stack);

  fault.addr = stack;
  fault.num = MCU_FAULT_MEM_UNKNOWN;

  if (mem_status & (1 << 5)) {
    fault.num = MCU_FAULT_MEMORY_FP_LAZY;
  }

  if (mem_status & (1 << 4)) {
    // this happens when an an interrupt happens and the hardware is unable to stack the
    // PSP with the hw stack frame when this happens the stack->pc and stack->lr values
    // will not be valid (they weren't stacked)
    fault.num = MCU_FAULT_MEMORY_STACKING;
  }

  if (mem_status & (1 << 3)) {
    // This fault is chainged to the handler
    fault.num = MCU_FAULT_MEMORY_UNSTACKING;
  }

  if (mem_status & (1 << 1)) {
    fault.num = MCU_FAULT_MEMORY_ACCESS_VIOLATION;
  }

  if (mem_status & (1 << 0)) {
    fault.num = MCU_FAULT_MEMORY_EXEC_VIOLATION;
  }

  if (mem_status & (1 << 7)) {
    fault.addr = (void *)SCB->MMFAR;
  }

  fault.pc = (void *)stack->pc;
  fault.caller = (void *)stack->lr;
  fault.handler_pc = (void *)handler_stack->pc;
  fault.handler_caller = (void *)handler_stack->lr;

  //! \todo This should always send the PC and the link register of the offending
  //! instructions
  cortexm_fault_event_handler(&fault);
}

void cortexm_busfault_handler() MCU_WEAK;
void cortexm_busfault_handler() {
  register void *handler_stack;
  asm volatile("MRS %0, msp\n\t" : "=r"(handler_stack));
  register u32 status;
  status = SCB->CFSR;
  SCB->CFSR = status; // clear the bits (by writing one)
  busfault_handler(status >> 8, handler_stack);
}

void busfault_handler(u32 bus_status, hw_stack_frame_t *handler_stack) {
  fault_t fault;
  hw_stack_frame_t *stack;
  cortexm_get_thread_stack_ptr((void **)&stack); // Clear the fault

  fault.addr = (void *)0xFFFFFFFF;
  fault.num = MCU_FAULT_BUS_UNKNOWN;

  if (bus_status & (1 << 7)) {
    fault.addr = (void *)SCB->BFAR;
  }

  if (bus_status & (1 << 4)) {
    fault.num = MCU_FAULT_BUS_STACKING;
    fault.addr = stack;
  }

  if (bus_status & (1 << 3)) {
    fault.num = MCU_FAULT_BUS_UNSTACKING;
    fault.addr = stack;
  }

  if (bus_status & (1 << 2)) {
    fault.num = MCU_FAULT_BUS_IMPRECISE;
  }

  if (bus_status & (1 << 1)) {
    fault.num = MCU_FAULT_BUS_PRECISE;
  }

  if (bus_status & (1 << 0)) {
    fault.num = MCU_FAULT_BUS_INSTRUCTION;
    fault.addr = (void *)stack->pc;
  }

  fault.pc = (void *)stack->pc;
  fault.caller = (void *)stack->lr;
  fault.handler_pc = (void *)handler_stack->pc;
  fault.handler_caller = (void *)handler_stack->lr;

  //! \todo This should always send the PC and the link register of the offending
  //! instructions
  cortexm_fault_event_handler(&fault);
}

void cortexm_usagefault_handler() MCU_WEAK;
void cortexm_usagefault_handler() {
  register void *handler_stack;
  asm volatile("MRS %0, msp\n\t" : "=r"(handler_stack));
  register u32 status;
  status = SCB->CFSR;
  SCB->CFSR = status; // clear the bits by writing one
  usagefault_handler(status >> 16, handler_stack);
}

void usagefault_handler(u32 usage_status, hw_stack_frame_t *handler_stack) {
  // Clear the fault
  fault_t fault;
  hw_stack_frame_t *stack;
  cortexm_get_thread_stack_ptr((void **)&stack);

  fault.addr = (void *)0xFFFFFFFF;
  fault.num = MCU_FAULT_USAGE_UNKNOWN;

  if (usage_status & (1 << 9)) {
    fault.num = MCU_FAULT_USAGE_DIVBYZERO;
  }

  if (usage_status & (1 << 8)) {
    fault.num = MCU_FAULT_USAGE_UNALIGNED;
  }

  if (usage_status & (1 << 3)) {
    fault.num = MCU_FAULT_USAGE_NO_COPROCESSOR;
  }

  if (usage_status & (1 << 2)) {
    fault.num = MCU_FAULT_USAGE_INVALID_PC;
    fault.addr = (void *)stack->pc;
  }

  if (usage_status & (1 << 1)) {
    fault.num = MCU_FAULT_USAGE_INVALID_STATE;
    fault.addr = (void *)stack->pc;
  }

  if (usage_status & (1 << 0)) {
    fault.num = MCU_FAULT_USAGE_UNDEFINED_INSTRUCTION;
    fault.addr = (void *)stack->pc;
  }

  fault.pc = (void *)stack->pc;
  fault.caller = (void *)stack->lr;
  fault.handler_pc = (void *)handler_stack->pc;
  fault.handler_caller = (void *)handler_stack->lr;

  cortexm_fault_event_handler(&fault);
}
