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



#include <cortex_m/task_flags.h>
#include <string.h>
#include <errno.h>
#include "mcu/mcu.h"
#include "mcu/debug.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "cortexm/task.h"

#define SYSTICK_MIN_CYCLES 10000


int task_rr_reload MCU_SYS_MEM;
int task_total MCU_SYS_MEM;
task_t * task_table MCU_SYS_MEM;
volatile int task_current MCU_SYS_MEM;


static void task_context_switcher();
static void priv_task_rr_reload(void * args) MCU_PRIV_EXEC_CODE;

static void system_reset(); //This is used if the OS process returns
void system_reset(){
	cortexm_svcall(cortexm_reset, NULL);
}


int task_init(int interval,
		void (*scheduler_function)(),
		void * system_memory,
		int system_memory_size){
	void * system_stack;
	hw_stack_frame_t * frame;
	int i;
	//Initialize the core system tick timer

	if ( task_table == NULL ){
		//The table must be allocated before the task manager is initialized
		return -1;
	}

	//Initialize the main process
	if ( system_memory == NULL ){
		system_memory = &_data;
	}

	system_stack = system_memory + system_memory_size;

	task_table[0].sp = system_stack - sizeof(hw_stack_frame_t);
	task_table[0].flags = TASK_FLAGS_EXEC | TASK_FLAGS_USED | TASK_FLAGS_ROOT;
	task_table[0].pid = 0;
	task_table[0].reent = _impure_ptr;
	task_table[0].global_reent = _global_impure_ptr;

	frame = (hw_stack_frame_t *)task_table[0].sp;
	frame->r0 = 0;
	frame->r1 = 0;
	frame->r2 = 0;
	frame->r3 = 0;
	frame->r12 = 0;
	frame->pc = ((uint32_t)scheduler_function);
	frame->lr = (uint32_t)system_reset;
	frame->psr = 0x21000000; //default PSR value
#if __FPU_USED != 0
	task_table[0].fpscr = FPU->FPDSCR;
#endif

	//enable use of PSP
	cortexm_set_thread_stack_ptr( (void*)task_table[0].sp );
	task_current = 0;

	//Set the interrupt priorities
	for(i=0; i <= mcu_config.irq_total; i++){
		mcu_core_set_nvic_priority( i, mcu_config.irq_middle_prio); //higher priority than the others
	}

	mcu_core_set_nvic_priority(SysTick_IRQn, mcu_config.irq_middle_prio+1); //lower priority so they don't interrupt the hardware
	mcu_core_set_nvic_priority(PendSV_IRQn, mcu_config.irq_middle_prio+1);
	mcu_core_set_nvic_priority(SVCall_IRQn, mcu_config.irq_middle_prio-1); //elevate this so it isn't interrupted by peripheral hardware
#if !defined MCU_NO_HARD_FAULT
	mcu_core_set_nvic_priority(HardFault_IRQn, 2);
#endif

	mcu_core_set_nvic_priority(DebugMonitor_IRQn, 0);
	mcu_core_set_nvic_priority(MemoryManagement_IRQn, 3);
	mcu_core_set_nvic_priority(BusFault_IRQn, 3);
	mcu_core_set_nvic_priority(UsageFault_IRQn, 3);

	//enable the FPU if it is in use
#if __FPU_USED != 0
	SCB->CPACR = (1<<20)|(1<<21)|(1<<22)|(1<<23); //allow full access to co-processor
	asm volatile("ISB");

	//FPU->FPCCR = (1<<31) | (1<<30); //set CONTROL<2> when FPU is used, enable lazy state preservation
	FPU->FPCCR = 0; //don't automatically save the FPU registers -- save them manually
#endif

	//Turn on the task timer (MCU implementation dependent)
	task_set_interval(interval);
	task_table[0].rr_time = task_rr_reload;

	cortexm_set_stack_ptr( (void*)&_top_of_stack ); //reset the handler stack pointer
	cortexm_enable_systick_irq();  //Enable context switching
	task_root_switch_context(NULL);
	return 0;
}

int task_set_interval(int interval){
	uint32_t reload;
	int core_tick_freq;
	reload = (mcu_board_config.core_cpu_freq * interval + 500) / 1000;
	if ( reload > (0x00FFFFFF) ){
		reload = (0x00FFFFFF);
	} else if ( reload < SYSTICK_MIN_CYCLES ){
		reload = SYSTICK_MIN_CYCLES;
	}
	core_tick_freq = mcu_board_config.core_cpu_freq / reload;
	SysTick->LOAD = reload;
	task_rr_reload = reload;
	SysTick->CTRL = SYSTICK_CTRL_ENABLE| //enable the timer
			SYSTICK_CTRL_CLKSROUCE; //Internal Clock CPU
	SCB->CCR = 0;
	return core_tick_freq;
}

int task_new_thread(void *(*p)(void*),
		void (*cleanup)(void*),
		void * arg,
		void * mem_addr,
		int mem_size,
		int pid){
	int tid;
	int thread_zero;
	void * stackaddr;
	new_task_t task;

	thread_zero = task_get_thread_zero(pid);
	if ( thread_zero < 0 ){
		//The PID is not valid
		return 0;
	}

	//Variable initialization
	stackaddr = mem_addr + mem_size;
	//Check the stack alignment
	if ( (unsigned int)stackaddr & 0x03 ){
		return -1;
	}

	//Initialize the task
	task.stackaddr = stackaddr;
	task.start = (uint32_t)p;
	task.stop = (uint32_t)cleanup;
	task.r0 = (uint32_t)arg;
	task.r1 = 0;
	task.pid = pid;
	//task.mem_size = mem_size;
	task.flags = TASK_FLAGS_USED |
			TASK_FLAGS_PARENT( task_get_parent(thread_zero) ) |
			TASK_FLAGS_IS_THREAD;
	task.reent = (struct _reent *)mem_addr;
	task.global_reent = task_table[ thread_zero ].global_reent;
	task.mem = &(task_table[ thread_zero ].mem);

	//Do a priv call while accessing the task table so there are no interruptions
	cortexm_svcall( (cortexm_svcall_t)task_root_new_task, &task);
	tid = task.tid;

	return tid;
}

void task_root_new_task(new_task_t * task){
	int i;
	hw_stack_frame_t * frame;

	for(i=1; i < task_get_total(); i++){
		if ( !task_used_asserted(i) ){
			//initialize the process stack pointer
			task_table[i].pid = task->pid;
			task_table[i].flags = task->flags;
			task_table[i].sp = task->stackaddr - sizeof(hw_stack_frame_t) - sizeof(sw_stack_frame_t);
			task_table[i].reent = task->reent;
			task_table[i].global_reent = task->global_reent;
			task_table[i].timer.t = 0;
			task_table[i].rr_time = task_rr_reload;
			memcpy(&(task_table[i].mem), task->mem, sizeof(task_memories_t));
#if __FPU_USED != 0
			task_table[i].fpscr = FPU->FPDSCR;
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



void task_root_del(int id){
	if ( (id < task_get_total() ) && (id >= 1)){
		task_deassert_used(id);
		task_deassert_exec(id);
	}
}


void task_root_resetstack(int id){
	//set the stack pointer to the original value
	task_table[id].sp = task_table[id].mem.data.addr + task_table[id].mem.data.size;
}

void * task_get_sbrk_stack_ptr(struct _reent * reent_ptr){
	int i;
	void * stackaddr;
	if ( task_table != NULL ){
		for(i=0; i < task_get_total(); i++){

			//If the reent and global reent are the same then this is the main thread
			if ( (task_table[i].reent == reent_ptr) && (task_table[i].global_reent == reent_ptr) ){

				//If the main thread is not in use, the stack is not valid
				if ( task_used_asserted(i) ){
					if ( (i == task_get_current()) ){
						//If the main thread is the current thread return the current stack
						cortexm_svcall(cortexm_get_thread_stack_ptr, &stackaddr);
						return stackaddr;
					} else {
						//Return the stack value from thread 0 if another thread is running
						return (void*)task_table[i].sp;
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
			if( pid == task_get_pid(i) && !task_isthread_asserted(i) ){
				return i;
			}
		}
	}
	return -1;
}

static void priv_task_tmr_rd(uint32_t * val){
	*val = task_rr_reload - SysTick->VAL;
}


uint64_t task_root_gettime(int tid){
	uint32_t val;
	if ( tid != task_get_current() ){
		return task_table[tid].timer.t + (task_rr_reload - task_table[tid].rr_time);
	} else {
		priv_task_tmr_rd(&val);
		return task_table[tid].timer.t + val;
	}
}


uint64_t task_gettime(int tid){
	uint32_t val;
	if ( tid != task_get_current() ){
		return task_table[tid].timer.t + (task_rr_reload - task_table[tid].rr_time);
	} else {
		cortexm_svcall((cortexm_svcall_t)priv_task_tmr_rd, &val);
		return task_table[tid].timer.t + val;
	}
}

void priv_task_rr_reload(void * args){
	task_table[task_get_current()].rr_time = task_rr_reload;
}

void task_reload(){
	cortexm_svcall(priv_task_rr_reload, 0);
}

/*! \details This function changes to another process.  It is executed
 * during the core systick timer and pend SV interrupts.
 */
void task_context_switcher(){
	int i;
	asm volatile ("MRS %0, psp\n\t" : "=r" (task_table[task_current].sp) );

#if __FPU_USED == 1
	void * fpu_stack;
	if( task_current != 0 ){
		//only do this if the task has used the FPU -- copy FPU registers to task table
		fpu_stack = task_table[task_current].fp + 32;
		asm volatile ("VMRS %0, fpscr\n\t" : "=r" (task_table[task_current].fpscr) );
		asm volatile ("mov r1, %0\n\t" : : "r" (fpu_stack) );
		asm volatile ("vstmdb r1!, {s0-s31}\n\t");
	}
#endif

	//disable task specific MPU regions
#if MPU_PRESENT || __MPU_PRESENT
	MPU->RBAR = 0x16;
	MPU->RASR = 0;
	MPU->RBAR = 0x17;
	MPU->RASR = 0;
	MPU->RBAR = 0x12;
	MPU->RASR = 0;
#endif

	cortexm_disable_interrupts(NULL);

	do {
		task_current++;
		if ( task_current == task_get_total() ){
			//The scheduler only uses OS mem -- disable the process MPU regions
			task_current = 0;
			if( task_table[0].rr_time < SYSTICK_MIN_CYCLES ){
				task_table[0].rr_time = task_rr_reload;
				task_table[0].timer.t += (task_rr_reload);
			}

			//see if all tasks have used up their RR time
			for(i=1; i < task_get_total(); i++){
				if ( task_exec_asserted(i) && (task_table[i].rr_time >= SYSTICK_MIN_CYCLES) ){
					break;
				}
			}

			//if all executing tasks have used up their RR time -- reload the RR time for executing tasks
			if ( i == task_get_total() ){
				for(i=1; i < task_get_total(); i++){
					if ( task_exec_asserted(i) ){
						task_table[i].timer.t += (task_rr_reload - task_table[i].rr_time);
						task_table[i].rr_time = task_rr_reload;
					}
				}
			}

			break;
		} else if ( task_exec_asserted(task_current) ){
			if ( (task_table[task_current].rr_time >= SYSTICK_MIN_CYCLES) || //is there time remaining on the RR
					task_isfifo_asserted(task_current) ){ //is this a FIFO task
				//Enable the MPU for the process code section
#if MPU_PRESENT || __MPU_PRESENT
				MPU->RBAR = (uint32_t)(task_table[task_current].mem.code.addr);
				MPU->RASR = (uint32_t)(task_table[task_current].mem.code.size);

				//Enable the MPU for the process data section
				MPU->RBAR = (uint32_t)(task_table[task_current].mem.data.addr);
				MPU->RASR = (uint32_t)(task_table[task_current].mem.data.size);
#endif
				break;
			}
		}
	} while(1);

	//Enable the MPU for the task stack guard
#if MPU_PRESENT || __MPU_PRESENT
	MPU->RBAR = (uint32_t)(task_table[task_current].mem.stackguard.addr);
	MPU->RASR = (uint32_t)(task_table[task_current].mem.stackguard.size);
#endif

	cortexm_enable_interrupts(NULL);

	_impure_ptr = task_table[task_current].reent;
	_global_impure_ptr = task_table[task_current].global_reent;

	if ( task_isfifo_asserted(task_current) ){
		//disable the systick interrupt (because this is a fifo task)
		cortexm_disable_systick_irq();
	} else {
		//init sys tick to the amount of time remaining
		SysTick->LOAD = task_table[task_current].rr_time;
		SysTick->VAL = 0; //force a reload
		//enable the systick interrupt
		cortexm_enable_systick_irq();
	}

#if __FPU_USED == 1
	//only do this if the task has used the FPU
	//task_load_fpu();
	if( task_current != 0 ){
		fpu_stack = task_table[task_current].fp;
		asm volatile ("VMSR fpscr, %0\n\t" : : "r" (task_table[task_current].fpscr) );
		asm volatile ("mov r1, %0\n\t" : : "r" (fpu_stack) );
		asm volatile ("vldm r1!, {s0-s31}\n\t");
	}
#endif

	asm volatile ("MSR psp, %0\n\t" : : "r" (task_table[task_current].sp) );
}

void task_root_switch_context(void * args){
	task_table[task_get_current()].rr_time = SysTick->VAL; //save the RR time from the SYSTICK
	SCB->ICSR |= (1<<28);
}

void _task_check_countflag(){
	if ( SysTick->CTRL & (1<<16) ){ //check the countflag
		task_table[task_current].rr_time = 0;
		task_context_switcher();
	}
}

void mcu_core_systick_handler() MCU_NAKED;
void mcu_core_systick_handler(){
	task_save_context();
	_task_check_countflag();
	task_load_context();
	task_return_context();
}

void mcu_core_pendsv_handler() MCU_NAKED;
void mcu_core_pendsv_handler(){
	task_save_context();
	task_context_switcher();
	task_load_context();
	task_return_context();
}


static void priv_task_restore(void * args) MCU_NAKED;
void priv_task_restore(void * args){
	asm volatile ("push {lr}\n\t");
	uint32_t pstack;

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
	cortexm_svcall(priv_task_restore, NULL);
}

int task_root_interrupt_call(void * args){
	uint32_t pstack;
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
			hw_frame = (hw_stack_frame_t *)(task_table[intr->tid].sp - sizeof(hw_stack_frame_t));
			task_table[intr->tid].sp = task_table[intr->tid].sp - (sizeof(hw_stack_frame_t) + sizeof(sw_stack_frame_t));
		}

		hw_frame->r0 = intr->arg[0];
		hw_frame->r1 = intr->arg[1];
		hw_frame->r2 = intr->arg[2];
		hw_frame->r3 = intr->arg[3];
		hw_frame->r12 = 0;
		hw_frame->pc = (uint32_t)intr->handler;
		hw_frame->lr = (uint32_t)task_restore;
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

uint32_t task_interrupt_stacksize(){
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


