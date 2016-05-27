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

#ifndef CORTEX_M_H_
#define CORTEX_M_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CORE_MAIN_RETURN 0xFFFFFFF9
#define CORE_THREAD_RETURN 0xFFFFFFFD

#define SYSTICK_CTRL_TICKINT (1<<1)

static inline void core_tick_enable_irq() MCU_ALWAYS_INLINE;
void core_tick_enable_irq(){
	SysTick->CTRL |= SYSTICK_CTRL_TICKINT;
}

static inline void core_tick_disable_irq() MCU_ALWAYS_INLINE;
void core_tick_disable_irq(){
	SysTick->CTRL &= ~SYSTICK_CTRL_TICKINT;
}

static inline void _mcu_core_priv_setvectortableaddr(void * addr) MCU_ALWAYS_INLINE;
void _mcu_core_priv_setvectortableaddr(void * addr){
	SCB->VTOR = (uint32_t)addr;
}



int core_set_interrupt_priority(int periph, int port, uint8_t priority);

#define TASK_MAIN_RETURN 0xFFFFFFF9
#define TASK_THREAD_RETURN 0xFFFFFFFD

#define SYSTICK_CTRL_TICKINT (1<<1)

/*
#if defined __FPU_USED
#undef __FPU_USED
#define __FPU_USED 0
#endif
*/


int core_fault_init(void (*handler)(int) );
void core_fault_reset_status();
int core_fault_debug();


#define r(x) register long x asm("lr")


#ifdef __cplusplus
}
#endif


#endif /* CORTEX_M_H_ */
