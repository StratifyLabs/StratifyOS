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

#ifndef _MCU_DEBUG_H_
#define _MCU_DEBUG_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mcu_debug_flags {
	MCU_DEBUG_ERROR = 1,
	MCU_DEBUG_WARNING = 2,
	MCU_DEBUG_INFO = 3,
	MCU_DEBUG_SYS = (1<<2),
	MCU_DEBUG_CORTEXM = (1<<3),
	MCU_DEBUG_DEVICE = (1<<4),
	MCU_DEBUG_AIO = (1<<5),
	MCU_DEBUG_CRT = (1<<6),
	MCU_DEBUG_DIRENT = (1<<7),
	MCU_DEBUG_MALLOC = (1<<8),
	MCU_DEBUG_MQUEUE = (1<<9),
	MCU_DEBUG_PROCESS = (1<<10),
	MCU_DEBUG_PTHREAD = (1<<11),
	MCU_DEBUG_SCHED = (1<<12),
	MCU_DEBUG_SCHEDULER = (1<<13),
	MCU_DEBUG_SEM = (1<<14),
	MCU_DEBUG_SIGNAL = (1<<15),
	MCU_DEBUG_FILESYSTEM = (1<<16),
	MCU_DEBUG_SOCKET = (1<<17),
	MCU_DEBUG_TIME = (1<<18),
	MCU_DEBUG_APPFS = (1<<19),
	MCU_DEBUG_LINK= (1<<20),
	MCU_DEBUG_UNISTD = (1<<21),
	MCU_DEBUG_USB = (1<<22),
	MCU_DEBUG_DEVFS = (1<<23),
	MCU_DEBUG_SGFX = (1<<24),
	MCU_DEBUG_SON = (1<<25),
	MCU_DEBUG_USER0 = (1<<26),
	MCU_DEBUG_USER1 = (1<<27),
	MCU_DEBUG_USER2 = (1<<28),
	MCU_DEBUG_USER3 = (1<<29),
	MCU_DEBUG_USER4 = (1<<30),
	MCU_DEBUG_USER5 = (1<<31)
};

#ifndef __link

#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "sos/fs/devfs.h"
#include "uart.h"

#if defined __debug
#define ___debug
#endif



#if !defined ___debug
#define mcu_debug_init() 0
#define mcu_debug_write_uart(x)
#define mcu_debug_user_printf(...)
#define mcu_debug_printf(...)
#define mcu_debug_root_printf(...)
#define mcu_debug_log_info(o_flags, format, ...)
#define mcu_debug_log_warning(o_flags, format, ...)
#define mcu_debug_log_error(o_flags, format, ...)
#define mcu_debug_log_fatal(o_flags, format, ...)
#define mcu_debug_trace_corrupt_memory()
#else
#define MCU_DEBUG 1
int mcu_debug_init();
void mcu_debug_root_write_uart(const char * buffer, int nbyte);
//int mcu_debug_user_printf(const char * format, ...);
//int mcu_debug_root_printf(const char * format, ...);
int mcu_debug_printf(const char * format, ...);
int mcu_debug_vprintf(const char * format, va_list args);

#define mcu_debug_root_printf mcu_debug_printf
#define mcu_debug_user_printf mcu_debug_printf

extern int malloc_is_memory_corrupt(struct _reent * reent);
#define mcu_debug_trace_corrupt_memory() do { mcu_debug_printf("%s():%d Memory ", __FUNCTION__, __LINE__); if( malloc_is_memory_corrupt(0) ){ mcu_debug_printf("Corrupt\n", __FUNCTION__, __LINE__); } else	{ mcu_debug_printf("OK\n", __FUNCTION__, __LINE__); } } while(0)

void mcu_debug_log_info(u32 o_flags, const char * format, ...);
void mcu_debug_log_warning(u32 o_flags, const char * format, ...);
void mcu_debug_log_error(u32 o_flags, const char * format, ...);
void mcu_debug_log_fatal(u32 o_flags, const char * format, ...);

#endif


#else

#define mcu_debug_printf(format, ...) printf(format, __VA_ARGS__)
#define mcu_debug_log_info(o_flags, format, ...) printf(format, __VA_ARGS__)
#define mcu_debug_log_warning(o_flags, format, ...) printf(format, __VA_ARGS__)
#define mcu_debug_log_error(o_flags, format, ...) printf(format, __VA_ARGS__)
#define mcu_debug_log_fatal(o_flags, format, ...) printf(format, __VA_ARGS__)

#endif

#define MCU_DEBUG_LINE_TRACE() mcu_debug_printf("%s():%d\n", __FUNCTION__, __LINE__)


#ifdef __cplusplus
}
#endif



#endif /* DEBUG_H_ */
