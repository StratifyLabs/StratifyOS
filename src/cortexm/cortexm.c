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

#include "cortexm_local.h"
#include "mcu/mcu.h"
#include "mcu/core.h"
#include "cortexm/mpu.h"

void cortexm_delay_systick(u32 ticks){
	u32 countdown = ticks;
	u32 start = cortexm_get_systick_value();
	u32 value;
	u32 end;

	if( SysTick->CTRL & 0x01 ){
		if( countdown > cortexm_get_systick_reload()/2){
			countdown = cortexm_get_systick_reload()/2;
		}

		if( countdown > start ){
			end = cortexm_get_systick_reload() - countdown + start;
			do {
				value = cortexm_get_systick_value();
			} while( (value < countdown) || (value > end) );
		} else {
			end = start - countdown;
			do {
				value = cortexm_get_systick_value();
			} while( (value > end) && (value < start) );
		}
	} else {
		volatile u32 i = 0;
		while(i < ticks){
			i++;
		}
	}
}

void cortexm_delay_us(u32 us){
	//ticks is the number of ticks in one microsecond
	u32 ticks = mcu_board_config.core_cpu_freq  / 1000000UL;
	cortexm_delay_systick(ticks*us);
}

void cortexm_delay_ms(u32 ms){
	int i;
	for(i=0; i < ms; i++){
		cortexm_delay_us(1000);
	}
}

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



void cortexm_reset_mode(){
	cortexm_disable_interrupts();
	mpu_disable();
	cortexm_disable_systick_irq();
}

void cortexm_set_privileged_mode(){
	register u32 control;
	control = __get_CONTROL();
	control &= ~0x01;
	__set_CONTROL(control);
}

void cortexm_set_unprivileged_mode(){
	register u32 control;
	control = __get_CONTROL();
	control |= 0x01;
	__set_CONTROL(control);
}

int cortexm_is_root_mode(){
	register u32 control;
	control = __get_CONTROL();
	return (control & 0x02) == 0;
}

void cortexm_set_thread_mode(){
	register u32 control;
	control = __get_CONTROL();
	control |= 0x02;
	__set_CONTROL(control);
}

void cortexm_svcall(cortexm_svcall_t call, void * args){
	asm volatile("SVC 0\n");
}

void mcu_core_svcall_handler() MCU_WEAK;
void mcu_core_svcall_handler(){
	register u32 * frame;
	register cortexm_svcall_t call;
	register void * args;
	asm volatile ("MRS %0, psp\n\t" : "=r" (frame) );
	call = (cortexm_svcall_t)frame[0];
	args = (void*)(frame[1]);
	//verify call is located in kernel text region
	if( ((u32)call >= (u32)&_text) && ((u32)call < (u32)&_etext) ){
		call(args);
	} else {
		//this needs to be a fault
	}
}

int cortexm_validate_callback(mcu_callback_t callback){
	if( ((u32)callback >= (u32)&_text) && ((u32)callback < (u32)&_etext) ){
		return 0;
	}
	return -1;
}

void cortexm_reset(void * args){
	NVIC_SystemReset();
}

void cortexm_disable_irq(s16 x){
	NVIC_DisableIRQ(x);
}

void cortexm_enable_irq(s16 x){
	NVIC_EnableIRQ(x);
}

void cortexm_disable_interrupts(){
	asm volatile ("cpsid i");
}

void cortexm_enable_interrupts(){
	asm volatile ("cpsie i");
}

void cortexm_get_stack_ptr(void * ptr){
	void ** ptrp = (void**)ptr;
	void * result=NULL;
	asm volatile ("MRS %0, msp\n\t" : "=r" (ptr) );
	*ptrp = result;
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

int cortexm_set_irq_priority(int irq, int prio, u32 o_events){


	if( o_events & MCU_EVENT_FLAG_SET_PRIORITY ){

		//calculate the relative priority (lower value is higher priority)
		prio = mcu_config.irq_middle_prio*2 - 1 - prio;

		//zero priority is reserved for exceptions -- lower value is higher priority
		if( prio < 4 ){
			prio = 4;
		}

		//ensure lowest priority (highest value) is not exceeded
		if( prio > (mcu_config.irq_middle_prio*2-1)){
			prio = mcu_config.irq_middle_prio*2-1;
		}

		//now set the priority in the NVIC
		mcu_core_set_nvic_priority(irq, prio);
	}

	return 0;
}

void cortexm_set_vector_table_addr(void * addr){
#if defined SCB
	SCB->VTOR = (uint32_t)addr;
	__DSB();
#endif
}

u32 cortexm_get_systick_value(){
	return SysTick->VAL;
}

u32 cortexm_get_systick_reload(){
	return SysTick->LOAD;
}

void cortexm_set_systick_reload(u32 value){
	SysTick->LOAD = value;
}

void cortexm_start_systick(){
	SysTick->CTRL = (1<<0) | (1<<2); //Internal Clock CPU and enable the timer
}

