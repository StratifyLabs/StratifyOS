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

/*! \addtogroup DELAY
 * @{
 *
 * \ingroup HWPL
 */

/*! \file */


#ifndef DELAY_CM_H_
#define DELAY_CM_H_


#include <stdint.h>

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
static inline void _delay_loop_1(uint32_t __count) MCU_ALWAYS_INLINE;


#endif


#ifdef __cplusplus
}
#endif


#endif /* DELAY_CM_H_ */

/*! @} */
