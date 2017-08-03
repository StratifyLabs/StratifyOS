/* Copyright 2011-2016 Tyler Gilbert;
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

#include "mcu/mcu.h"
#include "mcu/core.h"
#include "cortexm/cortexm.h"
#include "device/sys.h"


//This is 6 clock cycles per loop
void _delay_loop_1(uint32_t __count){
	asm volatile (
			"L_%=:"	"subs	%0, #1" "\n\t"
			"uxth	%0, %0" "\n\t"
			"cmp	%0, #0" "\n\t"
			"bne.n	L_%=" "\n\t"
			: "=r" (__count)
			: "0" (__count)
			);
}

static inline void _delay_us(uint32_t __us) MCU_ALWAYS_INLINE __attribute__((optimize("3")));

static inline void _delay_us(uint32_t __us){
	uint32_t __ticks;
	__ticks = mcu_board_config.core_cpu_freq / 1000000;
	__ticks *= __us;
	__ticks /= 6;
	_delay_loop_1(__ticks);
}

void cortexm_delay_us(u32 us){ _delay_us(us); }
void cortexm_delay_ms(u32 ms){ _delay_us(ms*1000); }

void cortexm_assign_zero_sum32(void * data, int count){
	u32 sum = 0;
	u32 * ptr = data;
	int i;
	for(i=0; i < count-1; i++){
		sum += ptr[i];
	}
	ptr[i] = (u32)(0 - sum);
}

void cortexm_assign_zero_sum8(void * data, int count){
	u8 sum = 0;
	u8 * ptr = data;
	int i;
	for(i=0; i < count-1; i++){
		sum += ptr[i];
	}
	ptr[i] = (u8)(0 - sum);
}

int cortexm_verify_zero_sum32(void * data, int count){
	u32 sum = 0;
	u32 * ptr = data;
	int i;
	for(i=0; i < count; i++){
		sum += ptr[i];
	}
	return sum == 0;
}

int cortexm_verify_zero_sum8(void * data, int count){
	u8 sum = 0;
	u8 * ptr = data;
	int i;
	for(i=0; i < count; i++){
		sum += ptr[i];
	}
	return sum == 0;
}


void cortexm_set_unprivileged_mode(){
	register uint32_t control;
	control = __get_CONTROL();
	control |= 0x01;
	__set_CONTROL(control);
}

void cortexm_set_thread_mode(){
	register uint32_t control;
	control = __get_CONTROL();
	control |= 0x02;
	__set_CONTROL(control);
}

void cortexm_svcall(cortexm_svcall_t call, void * args){
	asm volatile("SVC 0\n");
}

void mcu_core_svcall_handler(){
	register uint32_t * frame;
	register cortexm_svcall_t call;
	register void * args;
	asm volatile ("MRS %0, psp\n\t" : "=r" (frame) );
	call = (cortexm_svcall_t)frame[0];
	args = (void*)(frame[1]);
	call(args);
}

int cortexm_validate_callback(mcu_callback_t callback){
	return 0;
}

void cortexm_reset(void * args){
	NVIC_SystemReset();
}

void cortexm_disable_irq(void * x){
	NVIC_DisableIRQ((IRQn_Type)x);
}

void cortexm_enable_irq(void * x){
	NVIC_EnableIRQ((IRQn_Type)x);
}

void cortexm_disable_interrupts(void * args){
	asm volatile ("cpsid i");
}

void cortexm_enable_interrupts(void * args){
	asm volatile ("cpsie i");
}

void cortexm_get_stack_ptr(void * ptr){
	asm volatile ("MRS %0, msp\n\t" : "=r" (ptr) );
}

void cortexm_set_stack_ptr(void * ptr){
	asm volatile ("MSR msp, %0\n\t" : : "r" (ptr) );
}

void cortexm_get_thread_stack_ptr(void * ptr){
	void ** ptrp = (void**)ptr;
	void * result=NULL;
	asm volatile ("MRS %0, psp\n\t" : "=r" (result) );
	*ptrp = result;
}

void cortexm_set_thread_stack_ptr(void * ptr){
	asm volatile ("MSR psp, %0\n\t" : : "r" (ptr) );
}

int cortexm_set_irq_prio(int irq, int prio){

	prio = mcu_config.irq_middle_prio - prio;
	if( prio < 1 ){
		prio = 1;
	}

	if( prio > (mcu_config.irq_middle_prio*2-1)){
		prio = mcu_config.irq_middle_prio*2-1;
	}

	mcu_core_set_nvic_priority((IRQn_Type)irq, prio);

	return 0;
}

