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

#ifndef CORTEXM_LOCAL_H_
#define CORTEXM_LOCAL_H_

#include "mcu/arch.h"
#include "cortexm/cortexm.h"
#include "mcu/debug.h"

#define SYSTICK_CTRL_TICKINT (1<<1)

static inline void cortexm_enable_systick_irq() MCU_ALWAYS_INLINE;
void cortexm_enable_systick_irq(){
#if defined SysTick
	SysTick->CTRL |= SYSTICK_CTRL_TICKINT;
#endif
}

static inline void cortexm_disable_systick_irq() MCU_ALWAYS_INLINE;
void cortexm_disable_systick_irq(){
#if defined SysTick
	SysTick->CTRL &= ~SYSTICK_CTRL_TICKINT;
#endif
}

#endif /* CORTEXM_LOCAL_H_ */
