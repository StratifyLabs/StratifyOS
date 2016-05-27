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
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/task.h"
#include "task_flags.h"

static volatile uint32_t * volatile stack MCU_SYS_MEM; //static keeps this from being stored on the stack
static void system_reset(); //This is used if the OS process returns
void task_thread_stack_return();

int task_init_single(int (*initial_thread)(),
		void * system_memory,
		int system_memory_size){

	void * system_stack;
	hw_stack_frame_t * frame;
	int i;

	//Initialize the main process
	if ( system_memory == NULL ){
		system_memory = &_data;
	}
	system_stack = system_memory + system_memory_size - sizeof(hw_stack_frame_t);
	frame = (hw_stack_frame_t *)system_stack;
	frame->r0 = 0;
	frame->r1 = 0;
	frame->r2 = 0;
	frame->r3 = 0;
	frame->r12 = 0;
	frame->pc = (uint32_t)initial_thread;
	frame->lr = (uint32_t)system_reset;
	frame->psr = 0x21000000; //default PSR value

	//enable use of PSP
	_mcu_core_priv_set_thread_stack_ptr( system_stack );

	//Set the interrupt priorities
	for(i=0; i <= DEV_LAST_IRQ; i++){
		NVIC_SetPriority( i, DEV_MIDDLE_PRIORITY - 1 );
	}

	NVIC_SetPriority(SysTick_IRQn, DEV_MIDDLE_PRIORITY);
	NVIC_SetPriority(PendSV_IRQn, DEV_MIDDLE_PRIORITY);
	NVIC_SetPriority(SVCall_IRQn, DEV_MIDDLE_PRIORITY);

	//Turn on the SYSTICK timer and point to an empty interrupt
	core_tick_enable_irq();  //Enable context switching

	_mcu_core_priv_set_stack_ptr( (void*)&_top_of_stack ); //reset the handler stack pointer
	task_priv_switch_context(NULL);

	while(1);
	return 0;
}

void task_priv_switch_context(void * args){
	SCB->ICSR |= (1<<28); //Force a pendSV interrupt
}

void _mcu_core_pendsv_handler() MCU_NAKED;
void _mcu_core_pendsv_handler(){
	//The value 0xFFFFFFFD is pushed
#if __thumb2__ == 1
	asm("mvn.w r0, #2\n\t");
#else
	register int32_t r0 asm("r0");
	r0 = -2;
#endif
	asm volatile ("push {r0}\n\t");
	asm volatile ("pop {pc}\n\t"); //The value 0xFFFFFFFD is popped to do a thread stack return
}

void task_thread_stack_return(){
	*stack = TASK_THREAD_RETURN;
}

void system_reset(){
	mcu_core_privcall(_mcu_core_priv_reset, NULL);
}

int task_interrupt(task_interrupt_t * intr){
	//This function inserts the handler on the specified tasks stack
	return 0;
}

void task_restore(){}

void _mcu_core_systick_handler(){

}



