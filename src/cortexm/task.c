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

#include <string.h>
#include <errno.h>

#include "task_local.h"
#include "sos/sos.h"
#include "cortexm/task.h"

#define SYSTICK_MIN_CYCLES 10000

volatile s8 m_task_current_priority MCU_SYS_MEM;
static volatile u8 m_task_exec_count MCU_SYS_MEM;
int m_task_rr_reload MCU_SYS_MEM;
volatile int m_task_current MCU_SYS_MEM;
static void root_task_read_rr_timer(u32 * val);
static int set_systick_interval(int interval) MCU_ROOT_CODE;
static void switch_contexts();



static void system_reset(); //This is used if the OS process returns
void system_reset(){
	cortexm_svcall(cortexm_reset, NULL);
}


u8 task_get_exec_count(){ return m_task_exec_count; }
u8 task_get_total(){ return sos_board_config.task_total; }
s8 task_get_current_priority(){ return m_task_current_priority; }
void task_root_set_current_priority(s8 value){ m_task_current_priority = value; }

void task_root_elevate_current_priority(s8 value){
	cortexm_disable_interrupts();
	if( value > m_task_current_priority ){
		m_task_current_priority = value;
	}
	cortexm_enable_interrupts();
}

int task_init(int interval,
				  void (*scheduler_function)(),
				  void * system_memory,
				  int system_memory_size){
	void * system_stack;
	hw_stack_frame_t * frame;
	int i;


	//Initialize the main process
	if ( system_memory == NULL ){
		system_memory = &_data;
	}

	system_stack = system_memory + system_memory_size;

	sos_task_table[0].sp = system_stack - sizeof(hw_stack_frame_t);
	sos_task_table[0].flags = TASK_FLAGS_EXEC | TASK_FLAGS_USED | TASK_FLAGS_ROOT;
	sos_task_table[0].parent = 0;
	sos_task_table[0].priority = 0;
	sos_task_table[0].pid = 0;
	sos_task_table[0].reent = _impure_ptr;
	sos_task_table[0].global_reent = _global_impure_ptr;

	frame = (hw_stack_frame_t *)sos_task_table[0].sp;
	frame->r0 = 0;
	frame->r1 = 0;
	frame->r2 = 0;
	frame->r3 = 0;
	frame->r12 = 0;
	frame->pc = ((u32)scheduler_function);
	frame->lr = (u32)system_reset;
	frame->psr = 0x21000000; //default PSR value
#if __FPU_USED != 0
	sos_task_table[0].fpscr = FPU->FPDSCR;
#endif

	//enable use of PSP
	cortexm_set_thread_stack_ptr( (void*)sos_task_table[0].sp );
	m_task_current = 0;
	m_task_current_priority = 0;

	//Set the interrupt priorities
	for(i=0; i <= mcu_config.irq_total; i++){
		mcu_core_set_nvic_priority(i, mcu_config.irq_middle_prio*2-1); //mark as middle priority
	}

	mcu_core_set_nvic_priority(SysTick_IRQn, mcu_config.irq_middle_prio*2-1); //must be same priority as microsecond timer
	mcu_core_set_nvic_priority(PendSV_IRQn, mcu_config.irq_middle_prio*2-1);
	mcu_core_set_nvic_priority(SVCall_IRQn, mcu_config.irq_middle_prio*2-1); //elevate this so it isn't interrupted by peripheral hardware
#if !defined MCU_NO_HARD_FAULT
	mcu_core_set_nvic_priority(HardFault_IRQn, 2);
#endif

	//priority 1 is used for the WDT
	mcu_core_set_nvic_priority(DebugMonitor_IRQn, 0);
	mcu_core_set_nvic_priority(MemoryManagement_IRQn, 3);
	mcu_core_set_nvic_priority(BusFault_IRQn, 3);
	mcu_core_set_nvic_priority(UsageFault_IRQn, 3);

	mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_TASK_INIT, 0);

	//enable the FPU if it is in use
#if __FPU_USED != 0
	SCB->CPACR = (1<<20)|(1<<21)|(1<<22)|(1<<23); //allow full access to co-processor
	asm volatile("ISB");

	//FPU->FPCCR = (1<<31) | (1<<30); //set CONTROL<2> when FPU is used, enable lazy state preservation
	FPU->FPCCR = 0; //don't automatically save the FPU registers -- save them manually
#endif

	//this will causes crashes if it is moved after the context switching begins
	mcu_core_enable_cache();

	//Turn on the task timer (MCU implementation dependent)
	set_systick_interval(interval);
	sos_task_table[0].rr_time = m_task_rr_reload;
	cortexm_set_stack_ptr( (void*)&_top_of_stack ); //reset the handler stack pointer
	cortexm_enable_systick_irq();  //Enable context switching


	task_root_switch_context();
	return 0;
}

int set_systick_interval(int interval){
	u32 reload;
	int core_tick_freq;
	reload = (mcu_board_config.core_cpu_freq * interval + 500) / 1000;
	if ( reload > (0x00FFFFFF) ){
		reload = (0x00FFFFFF);
	} else if ( reload < SYSTICK_MIN_CYCLES ){
		reload = SYSTICK_MIN_CYCLES;
	}
	core_tick_freq = mcu_board_config.core_cpu_freq / reload;
	cortexm_set_systick_reload(reload);
	m_task_rr_reload = reload;
	cortexm_start_systick();
	SCB->CCR = 0;
	return core_tick_freq;
}


int task_create_thread(void *(*p)(void*),
							  void (*cleanup)(void*),
							  void * arg,
							  void * mem_addr,
							  int mem_size,
							  int pid){
	int thread_zero;
	void * stackaddr;
	new_task_t task;

	//valid validity of pid and stack
	thread_zero = task_get_thread_zero(pid);
	stackaddr = mem_addr + mem_size;
	if ( (thread_zero < 0) || ((u32)stackaddr & 0x03) ){
		//pid doesn't exist or stackaddr is misaligned
		return 0;
	}

	//Initialize the task
	task.stackaddr = stackaddr;
	task.start = (u32)p;
	task.stop = (u32)cleanup;
	task.r0 = (u32)arg;
	task.r1 = 0;
	task.pid = pid;
	task.flags = TASK_FLAGS_USED | TASK_FLAGS_THREAD;
	task.parent = task_get_parent(thread_zero);
	task.priority = 0;
	task.reent = mem_addr;
	task.global_reent = sos_task_table[ thread_zero ].global_reent;
	task.mem = (void*)&(sos_task_table[ thread_zero ].mem);

	//Do a priv call while accessing the task table so there are no interruptions
	cortexm_svcall( (cortexm_svcall_t)task_root_new_task, &task);
	return task.tid;
}

void task_root_new_task(new_task_t * task){
	int i;
	hw_stack_frame_t * frame;

	for(i=1; i < task_get_total(); i++){
		if ( !task_used_asserted(i) ){
			//initialize the process stack pointer
			sos_task_table[i].pid = task->pid;
			sos_task_table[i].parent = task->parent;
			sos_task_table[i].flags = task->flags;
			sos_task_table[i].sp = task->stackaddr - sizeof(hw_stack_frame_t) - sizeof(sw_stack_frame_t);
			sos_task_table[i].reent = task->reent;
			sos_task_table[i].global_reent = task->global_reent;
			sos_task_table[i].timer.t = 0;
			sos_task_table[i].rr_time = m_task_rr_reload;
			memcpy((void*)&(sos_task_table[i].mem), task->mem, sizeof(task_memories_t));
#if __FPU_USED != 0
			sos_task_table[i].fpscr = FPU->FPDSCR;
#endif
			break;
		}
	}
	if ( i == task_get_total() ){
		task->tid = 0;
	} else {
		//Initialize the stack frame
		frame = (hw_stack_frame_t *)(task->stackaddr - sizeof(hw_stack_frame_t));
		frame->r0 = task->r0;
		frame->r1 = task->r1;
		frame->r2 = 0;
		frame->r3 = 0;
		frame->r12 = 0;
		frame->pc = task->start;
		frame->lr = task->stop;
		frame->psr = 0x21000000; //default PSR value

		task->tid = i;
	}
}

void task_root_delete(int id){
	if ( (id < task_get_total() ) && (id >= 1)){
		task_deassert_used(id);
		task_deassert_exec(id);
	}
}


void task_root_resetstack(int id){
	//set the stack pointer to the original value
	sos_task_table[id].sp = mpu_addr((u32)sos_task_table[id].mem.data.addr) + mpu_size((u32)sos_task_table[id].mem.data.size) -
			(sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t));
	if( id == task_get_current() ){ //make effective now
		cortexm_set_thread_stack_ptr((void*)sos_task_table[id].sp);
	}

}

void * task_get_sbrk_stack_ptr(struct _reent * reent_ptr){
	int i;
	void * stackaddr;
	if ( sos_task_table != NULL ){
		for(i=0; i < task_get_total(); i++){

			//If the reent and global reent are the same then this is the main thread
			if ( (sos_task_table[i].reent == reent_ptr) && (sos_task_table[i].global_reent == reent_ptr) ){

				//If the main thread is not in use, the stack is not valid
				if ( task_used_asserted(i) ){
					if ( (i == task_get_current()) ){
						//If the main thread is the current thread return the current stack
						cortexm_svcall(cortexm_get_thread_stack_ptr, &stackaddr);
						return stackaddr;
					} else {
						//Return the stack value from thread 0 if another thread is running
						return (void*)sos_task_table[i].sp;
					}
				} else {
					//The main thread is not in use, so there is no valid stack value
					return NULL;
				}
			}
		}
	}

	//No task table (or not matching reent structure) so no valid stack value
	return NULL;
}

int task_get_thread_zero(int pid){
	int i;
	for(i=0; i < task_get_total(); i++){
		if ( task_used_asserted(i) ){
			if( pid == task_get_pid(i) && !task_thread_asserted(i) ){
				return i;
			}
		}
	}
	return -1;
}

static void root_task_read_rr_timer(u32 * val){
	*val = m_task_rr_reload - SysTick->VAL;
}


u64 task_root_gettime(int tid){
	u32 val;
	if ( tid != task_get_current() ){
		return sos_task_table[tid].timer.t + (m_task_rr_reload - sos_task_table[tid].rr_time);
	} else {
		root_task_read_rr_timer(&val);
		return sos_task_table[tid].timer.t + val;
	}
}


u64 task_gettime(int tid){
	u32 val;
	if ( tid != task_get_current() ){
		return sos_task_table[tid].timer.t + (m_task_rr_reload - sos_task_table[tid].rr_time);
	} else {
		cortexm_svcall((cortexm_svcall_t)root_task_read_rr_timer, &val);
		return sos_task_table[tid].timer.t + val;
	}
}

void switch_contexts(){
	int i;
	//Save the PSP to the current task's stack pointer
	asm volatile ("MRS %0, psp\n\t" : "=r" (sos_task_table[m_task_current].sp) );

#if __FPU_USED == 1
	volatile void * fpu_stack;
	if( m_task_current != 0 ){
		//only do this if the task has used the FPU -- copy FPU registers to task table
		fpu_stack = sos_task_table[m_task_current].fp + 32;
		asm volatile ("VMRS %0, fpscr\n\t" : "=r" (sos_task_table[m_task_current].fpscr) );
		asm volatile ("mov r1, %0\n\t" : : "r" (fpu_stack) );
		asm volatile ("vstmdb r1!, {s0-s31}\n\t");
	}
#endif

	do {
		m_task_current++;
		if ( m_task_current == task_get_total() ){
			//The scheduler only uses OS mem -- disable the process MPU regions
			m_task_current = 0;
			if( sos_task_table[0].rr_time < SYSTICK_MIN_CYCLES ){
				sos_task_table[0].rr_time = m_task_rr_reload;
				sos_task_table[0].timer.t += (m_task_rr_reload);
			}

			//see if all tasks have used up their RR time
			for(i=1; i < task_get_total(); i++){
				if ( task_exec_asserted(i) && (sos_task_table[i].rr_time >= SYSTICK_MIN_CYCLES) ){
					break;
				}
			}

			//if all executing tasks have used up their RR time -- reload the RR time for executing tasks
			if ( i == task_get_total() ){
				for(i=1; i < task_get_total(); i++){
					if ( task_exec_asserted(i) ){
						sos_task_table[i].timer.t += (m_task_rr_reload - sos_task_table[i].rr_time);
						sos_task_table[i].rr_time = m_task_rr_reload;
					}
				}
			}

			break;
		} else if ( task_exec_asserted(m_task_current) ){
			if ( (sos_task_table[m_task_current].rr_time >= SYSTICK_MIN_CYCLES) || //is there time remaining on the RR
				  task_fifo_asserted(m_task_current) ){ //is this a FIFO task
				//check to see if task is low on memory -- kill if necessary?

				break;
			}
		}
	} while(1);

	//Enable the MPU for the task stack guard
#if MPU_PRESENT || __MPU_PRESENT
	//Enable the MPU for the process code section
	MPU->RBAR = (u32)(sos_task_table[m_task_current].mem.code.addr);
	MPU->RASR = (u32)(sos_task_table[m_task_current].mem.code.size);

	//Enable the MPU for the process data section
	MPU->RBAR = (u32)(sos_task_table[m_task_current].mem.data.addr);
	MPU->RASR = (u32)(sos_task_table[m_task_current].mem.data.size);

	MPU->RBAR = (u32)(sos_task_table[m_task_current].mem.stackguard.addr);
	MPU->RASR = (u32)(sos_task_table[m_task_current].mem.stackguard.size);

	//disable MPU for ROOT tasks
	if( task_root_asserted(m_task_current) ){
		mpu_disable();
	} else {
		mpu_enable();
	}

#endif

	_impure_ptr = sos_task_table[m_task_current].reent;
	_global_impure_ptr = sos_task_table[m_task_current].global_reent;

	if ( task_fifo_asserted(m_task_current) ){
		//disable the systick interrupt (because this is a fifo task)
		cortexm_disable_systick_irq();
	} else {
		//init sys tick to the amount of time remaining
		SysTick->LOAD = sos_task_table[m_task_current].rr_time;
		SysTick->VAL = 0; //force a reload
		//enable the systick interrupt
		cortexm_enable_systick_irq();
	}

#if __FPU_USED == 1
	//only do this if the task has used the FPU
	//task_load_fpu();
	if( m_task_current != 0 ){
		fpu_stack = sos_task_table[m_task_current].fp;
		asm volatile ("VMSR fpscr, %0\n\t" : : "r" (sos_task_table[m_task_current].fpscr) );
		asm volatile ("mov r1, %0\n\t" : : "r" (fpu_stack) );
		asm volatile ("vldm r1!, {s0-s31}\n\t");
	}
#endif

	//write the new task's stack pointer to the PSP
	asm volatile ("MSR psp, %0\n\t" : : "r" (sos_task_table[m_task_current].sp) );
}

void task_root_switch_context(){
	sos_task_table[task_get_current()].rr_time = SysTick->VAL; //save the RR time from the SYSTICK
	SCB->ICSR |= (1<<28); //set the pend SV interrupt pending -- causes mcu_core_pendsv_handler() to execute when current interrupt exits
}

void task_check_count_flag(){
	if ( SysTick->CTRL & (1<<16) ){ //check the countflag
		sos_task_table[m_task_current].rr_time = 0;
		switch_contexts();
	}
}

void mcu_core_systick_handler() MCU_NAKED MCU_WEAK;
void mcu_core_systick_handler(){
	task_save_context();
	task_check_count_flag();
	task_load_context();
	task_return_context();
}

//Weak is needed for overriding when building bootloader
void mcu_core_pendsv_handler() MCU_NAKED MCU_WEAK;
void mcu_core_pendsv_handler(){
	task_save_context();

	//disable interrupts -- Re-entrant scheduler issue #130
	m_task_exec_count = 0;
	cortexm_disable_interrupts();
	int i;
	for(i=1; i < task_get_total(); i++){

		if( task_enabled_active_not_stopped(i) && //enabled, active, and not stopped
			 ( task_get_priority(i) == task_get_current_priority() ) ){ //task is equal to the currently executing priority
			//Enable process execution for highest active priority tasks
			task_assert_exec(i);
			m_task_exec_count++;
		} else {
			//Disable process execution for lower priority tasks
			task_deassert_exec(i);
		}
	}

	//enable interrupts -- Re-entrant scheduler issue
	cortexm_enable_interrupts();

	//switch contexts if current task is not executing or it wants to yield
	if(  (task_get_current()) == 0 || //always switch away from task zero if requested
		  (task_exec_asserted(task_get_current()) == 0) || //checks if current task is NOT running
		  task_yield_asserted(task_get_current()) ){ //checks if current task requested a context switch
		task_deassert_yield(task_get_current());
		switch_contexts();
	}

	task_load_context();
	task_return_context();
}


static void root_task_restore(void * args) MCU_NAKED;
void root_task_restore(void * args){
	asm volatile ("push {lr}\n\t");
	u32 pstack;

	//discard the current HW stack by adjusting the PSP up by sizeof(hw_stack_frame_t) --sw_stack_frame_t is same size
	pstack = __get_PSP();

	__set_PSP(pstack + sizeof(hw_stack_frame_t));

	//Load the software context that is on the stack from the pre-interrupted task
	task_load_context();
	//This function will now return to the original execution stack
	asm volatile ("pop {pc}\n\t");
}

void task_restore(){
	//handlers inserted with task_interrupt() must call this function when the task completes in order to restore the stack
	cortexm_svcall(root_task_restore, NULL);
}

int task_root_interrupt_call(void * args){
	u32 pstack;
	task_interrupt_t * intr = (task_interrupt_t*)args;
	hw_stack_frame_t * hw_frame;

	if ( task_enabled(intr->tid) ){
		if ( intr->tid == task_get_current() ){
			pstack = __get_PSP();
			__set_PSP(pstack - sizeof(hw_stack_frame_t));
			hw_frame = (hw_stack_frame_t *)__get_PSP(); //frame now points to the new HW stack

			//current tid is interrupting tid so software stack is not changed

		} else {
			//Since this is another task, the current PSP is not touched
			hw_frame = (hw_stack_frame_t *)(sos_task_table[intr->tid].sp - sizeof(hw_stack_frame_t));
			sos_task_table[intr->tid].sp = sos_task_table[intr->tid].sp - (sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t));
		}

		hw_frame->r0 = intr->arg[0];
		hw_frame->r1 = intr->arg[1];
		hw_frame->r2 = intr->arg[2];
		hw_frame->r3 = intr->arg[3];
		hw_frame->r12 = 0;
		hw_frame->pc = (u32)intr->handler;
		hw_frame->lr = (u32)task_restore;
		hw_frame->psr = 0x21000000; //default PSR value
	}

	if ( intr->sync_callback != NULL ){
		intr->sync_callback(intr->sync_callback_arg);
	}

	if ( intr->tid != task_get_current() ){
		//this task is not the currently executing task
		return 1;
	}
	return 0;
}

u32 task_interrupt_stacksize(){
	return sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t);
}


void task_root_interrupt(void * args){
	task_save_context(); //save the current software context
	//This function inserts the handler on the specified task's stack

	if( task_root_interrupt_call(args) ){
		task_load_context(); //the context needs to be loaded for the current task -- otherwise it is loaded by the switcher
	}
}

int task_interrupt(task_interrupt_t * intr){
	if ( intr->tid < task_get_total() ){
		cortexm_svcall(task_root_interrupt, intr);
		return 0;
	}
	return -1;
}


