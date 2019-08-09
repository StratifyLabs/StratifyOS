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

#ifndef TASK_LOCAL_H_
#define TASK_LOCAL_H_

#include "cortexm_local.h"
#include "cortexm/task.h"
#include "cortexm/mpu.h"

#define SYSTICK_CTRL_ENABLE (1<<0)
#define SYSTICK_CTRL_CLKSROUCE (1<<2)
#define SYSTICK_CTRL_COUNTFLAG (1<<16)

typedef struct {
	//u32 exc_return;
	u32 r0;
	u32 r1;
	u32 r2;
	u32 r3;
	u32 r12;
	u32 lr;
	u32 pc;
	u32 psr;

} hw_stack_frame_t;

typedef struct MCU_PACK {
	u32 r4;
	u32 r5;
	u32 r6;
	u32 r7;
	u32 r8;
	u32 r9;
	u32 r10;
	u32 r11;
} sw_stack_frame_t;

#define TASK_DEBUG 0

#define task_debug(...) do { if ( TASK_DEBUG == 1 ){ mcu_debug_user_printf("%s:", __func__); mcu_debug_user_printf(__VA_ARGS__); } } while(0)

extern int task_total MCU_SYS_MEM;
extern task_t * task_table MCU_SYS_MEM;
extern volatile int m_task_current MCU_SYS_MEM;

typedef struct {
	int tid;
	int pid;
    u8 flags;
    s8 priority;
    u16 parent;
	struct _reent * reent;
	struct _reent * global_reent;
	task_memories_t * mem;
	void * stackaddr;
	u32 start;
	u32 stop;
	u32 r0;
	u32 r1;
	u32 r2;
	u32 r3;
} new_task_t;

void task_svcall_new_task(new_task_t * task);

static inline void task_save_context() MCU_ALWAYS_INLINE;
void task_save_context(){
	asm volatile ("MRS r1, psp\n\t"
			"STMDB r1!, {r4-r11}\n\t"
			"MSR psp, r1\n\t");
}

static inline void task_load_context() MCU_ALWAYS_INLINE;
void task_load_context(){
	asm volatile ("MRS r1, psp\n\t"
			"LDMFD r1!, {r4-r11}\n\t"
			"MSR psp, r1\n\t");
}

static inline void task_return_context() MCU_ALWAYS_INLINE;
void task_return_context(){
	asm volatile ("mvn.w r0, #2\n\t"); //use non-floating state from PSP and execute from PSP
	asm volatile ("bx r0\n\t");
}



#endif /* TASK_LOCAL_H_ */
