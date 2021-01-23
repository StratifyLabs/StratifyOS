// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef CORTEXM_LOCAL_H_
#define CORTEXM_LOCAL_H_

#include "cortexm/cortexm.h"
#include "sos/arch.h"
#include "sos/debug.h"

#define SYSTICK_CTRL_TICKINT (1 << 1)

static inline void cortexm_enable_systick_irq() MCU_ALWAYS_INLINE;
void cortexm_enable_systick_irq() {
#if defined SysTick
  SysTick->CTRL |= SYSTICK_CTRL_TICKINT;
#endif
}

static inline void cortexm_disable_systick_irq() MCU_ALWAYS_INLINE;
void cortexm_disable_systick_irq() {
#if defined SysTick
  SysTick->CTRL &= ~SYSTICK_CTRL_TICKINT;
#endif
}

#endif /* CORTEXM_LOCAL_H_ */
