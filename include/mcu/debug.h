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

#ifndef _MCU_DEBUG_H_
#define _MCU_DEBUG_H_

#include <stdio.h>
#include <string.h>

#ifndef __link


#ifdef __cplusplus
extern "C" {
#endif


#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "sos/fs/devfs.h"
#include "uart.h"


#if !defined ___debug
#define mcu_debug_init() 0
#define mcu_debug_write_uart(x)
#define mcu_debug_user_printf(...)
#define mcu_debug_printf(...)
#define mcu_debug_root_printf(...)
#else
#define MCU_DEBUG 1
int mcu_debug_init();
void mcu_debug_root_write_uart(const char * buffer, int nbyte);


int mcu_debug_user_printf(const char * format, ...);
int mcu_debug_root_printf(const char * format, ...);

#define mcu_debug_printf mcu_debug_root_printf


#endif



#ifdef __cplusplus
}
#endif

#endif



#endif /* DEBUG_H_ */
