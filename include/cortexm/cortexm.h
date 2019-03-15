/* Copyright 2011-2018 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef MCU_CORTEXM_H_
#define MCU_CORTEXM_H_

#include "mcu/types.h"

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
void cortexm_get_stack_ptr(void * ptr) MCU_ROOT_CODE;
void cortexm_set_stack_ptr(void * ptr) MCU_ROOT_CODE;
void cortexm_get_thread_stack_ptr(void * ptr) MCU_ROOT_CODE;
void cortexm_set_thread_stack_ptr(void * ptr) MCU_ROOT_CODE;
int cortexm_validate_callback(mcu_callback_t callback) MCU_ROOT_CODE;
int cortexm_set_irq_priority(int irq, int prio, u32 o_events) MCU_ROOT_CODE;

void cortexm_set_systick_reload(u32 value);
void cortexm_start_systick();
u32 cortexm_get_systick_value();
u32 cortexm_get_systick_reload();


#define CORTEXM_ZERO_SUM32_COUNT(x) (sizeof(x)/sizeof(u32))
#define CORTEXM_ZERO_SUM8_COUNT(x) (sizeof(x))

void cortexm_assign_zero_sum32(void * data, int size);
void cortexm_assign_zero_sum8(void * data, int size);
int cortexm_verify_zero_sum32(void * data, int size);
int cortexm_verify_zero_sum8(void * data, int size);

typedef void (*cortexm_svcall_t)(void*);
void cortexm_svcall(cortexm_svcall_t call, void * args) __attribute__((optimize("1")));

void cortexm_reset_mode() MCU_ROOT_CODE;
void cortexm_set_privileged_mode() MCU_ROOT_CODE;
void cortexm_set_unprivileged_mode() MCU_ROOT_CODE;
void cortexm_set_thread_mode() MCU_ROOT_CODE;
int cortexm_is_root_mode();

void cortexm_delay_us(u32 us);
void cortexm_delay_ms(u32 ms);
void cortexm_delay_systick(u32 ticks);

void cortexm_set_vector_table_addr(void * addr);

void cortexm_wdtfault_handler(void * stack);


#define r(x) register long x asm("lr")

#endif

#ifdef __cplusplus
}
#endif


#endif /* MCU_CORTEXM_H_ */
