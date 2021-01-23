// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup DELAY
 * @{
 *
 * \ingroup HWPL
 */

/*! \file */


#ifndef DELAY_CM_H_
#define DELAY_CM_H_


#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef F_CPU
#define F_CPU 72000000UL
#endif

#if defined __lpc8xx || defined __lpc82x
static inline void _delay_ms(uint32_t __ms){}

static inline void _delay_us(uint32_t __us){}
#else
//user F_CPU to create millisecond and microsecond delay loops
static inline void _delay_loop_1(u32 __count) MCU_ALWAYS_INLINE;


#endif


#ifdef __cplusplus
}
#endif


#endif /* DELAY_CM_H_ */

/*! @} */
