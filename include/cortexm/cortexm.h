// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef CORTEXM_CORTEXM_H_
#define CORTEXM_CORTEXM_H_

#include <sdk/types.h>

#include "mcu/arch/cmsis/cmsis_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __link

int cortexm_sleep(int level) MCU_ROOT_CODE;

void cortexm_enable_interrupts() MCU_ROOT_CODE;
void cortexm_disable_interrupts() MCU_ROOT_CODE;
void cortexm_enable_irq(s16 x) MCU_ROOT_CODE;
void cortexm_disable_irq(s16 x) MCU_ROOT_CODE;
void cortexm_reset(void * args) MCU_ROOT_CODE;
void cortexm_get_stack_ptr(void ** ptr) MCU_ROOT_CODE;
void cortexm_set_stack_ptr(void * ptr) MCU_ROOT_CODE;
void cortexm_get_thread_stack_ptr(void ** ptr) MCU_ROOT_CODE;
void cortexm_set_thread_stack_ptr(void * ptr) MCU_ROOT_CODE;
int cortexm_validate_callback(mcu_callback_t callback) MCU_ROOT_CODE;
int cortexm_set_irq_priority(int irq, int prio, u32 o_events) MCU_ROOT_CODE;

void cortexm_svcall_get_thread_stack_ptr(void * ptr) MCU_ROOT_CODE;

void cortexm_set_systick_reload(u32 value) MCU_ROOT_CODE;
void cortexm_start_systick() MCU_ROOT_CODE;
u32 cortexm_get_systick_value() MCU_ROOT_CODE;
u32 cortexm_get_systick_reload() MCU_ROOT_CODE;


#define CORTEXM_ZERO_SUM32_COUNT(x) (sizeof(x)/sizeof(u32))
#define CORTEXM_ZERO_SUM8_COUNT(x) (sizeof(x))

void cortexm_assign_zero_sum32(void * data, int size);
void cortexm_assign_zero_sum8(void * data, int size);
int cortexm_verify_zero_sum32(void * data, int size);
int cortexm_verify_zero_sum8(void * data, int size);

typedef void (*cortexm_svcall_t)(void*);
void cortexm_svcall(cortexm_svcall_t call, void * args) __attribute__((optimize("1")));

void cortexm_svcall_handler() MCU_ROOT_CODE;
void cortexm_initialize_heap() MCU_ROOT_CODE;

void cortexm_reset_mode() MCU_ROOT_CODE;
void cortexm_set_privileged_mode() MCU_ROOT_CODE;
void cortexm_set_unprivileged_mode() MCU_ROOT_CODE;
void cortexm_set_thread_mode() MCU_ROOT_CODE;
static inline int cortexm_is_root_mode() {
  register u32 control;
  control = __get_CONTROL();
  return (control & 0x02) == 0;
}

void cortexm_delay_us(u32 us) MCU_ROOT_EXEC_CODE;
void cortexm_delay_ms(u32 ms) MCU_ROOT_EXEC_CODE;
void cortexm_delay_systick(u32 ticks) MCU_ROOT_EXEC_CODE;

void cortexm_set_vector_table_addr(void *addr) MCU_ROOT_EXEC_CODE;
u32 cortexm_get_vector_table_addr() MCU_ROOT_EXEC_CODE;

void cortexm_wdtfault_handler(void *stack) MCU_ROOT_EXEC_CODE;
void cortexm_reset_handler() __attribute__((section(".reset_vector")));
void cortexm_nmi_handler() MCU_ROOT_EXEC_CODE;
void cortexm_debug_monitor_handler() MCU_ROOT_EXEC_CODE;

//This is used to ensure that privileged code executes from start to finish (argument validation cannot be bypassed)
extern cortexm_svcall_t cortexm_svcall_validation MCU_SYS_MEM;
#define CORTEXM_SVCALL_ENTER() (cortexm_svcall_validation = (cortexm_svcall_t)__FUNCTION__)


#define CORTEXM_DECLARE_LINK_REGISTER(x) register long x asm("lr")

#endif

#ifdef __cplusplus
}
#endif

#endif /* CORTEXM_CORTEXM_H_ */
