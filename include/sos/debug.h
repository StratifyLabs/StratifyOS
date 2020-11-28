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

#ifndef _SOS_DEBUG_H_
#define _SOS_DEBUG_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

enum sos_debug_flags {
  SOS_DEBUG_ERROR = 1,
  SOS_DEBUG_WARNING = 2,
  SOS_DEBUG_INFO = 3,
  SOS_DEBUG_SYS = (1 << 2),
  SOS_DEBUG_CORTEXM = (1 << 3),
  SOS_DEBUG_DEVICE = (1 << 4),
  SOS_DEBUG_AIO = (1 << 5),
  SOS_DEBUG_CRT = (1 << 6),
  SOS_DEBUG_DIRENT = (1 << 7),
  SOS_DEBUG_MALLOC = (1 << 8),
  SOS_DEBUG_MQUEUE = (1 << 9),
  SOS_DEBUG_PROCESS = (1 << 10),
  SOS_DEBUG_PTHREAD = (1 << 11),
  SOS_DEBUG_SCHED = (1 << 12),
  SOS_DEBUG_SCHEDULER = (1 << 13),
  SOS_DEBUG_SEM = (1 << 14),
  SOS_DEBUG_SIGNAL = (1 << 15),
  SOS_DEBUG_FILESYSTEM = (1 << 16),
  SOS_DEBUG_SOCKET = (1 << 17),
  SOS_DEBUG_TIME = (1 << 18),
  SOS_DEBUG_APPFS = (1 << 19),
  SOS_DEBUG_LINK = (1 << 20),
  SOS_DEBUG_UNISTD = (1 << 21),
  SOS_DEBUG_USB = (1 << 22),
  SOS_DEBUG_DEVFS = (1 << 23),
  SOS_DEBUG_SGFX = (1 << 24),
  SOS_DEBUG_SON = (1 << 25),
  SOS_DEBUG_USER0 = (1 << 26),
  SOS_DEBUG_USER1 = (1 << 27),
  SOS_DEBUG_USER2 = (1 << 28),
  SOS_DEBUG_USER3 = (1 << 29),
  SOS_DEBUG_USER4 = (1 << 30),
  SOS_DEBUG_USER5 = (1 << 31)
};

#ifndef __link

#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "sos/fs/devfs.h"

#if defined __debug
#define ___debug
#endif

#if !defined ___debug
#define sos_debug_user_printf(...)
#define sos_debug_printf(...)
#define sos_debug_root_printf(...)
#define sos_debug_log_info(o_flags, format, ...)
#define sos_debug_log_warning(o_flags, format, ...)
#define sos_debug_log_error(o_flags, format, ...)
#define sos_debug_log_fatal(o_flags, format, ...)
#define sos_debug_trace_corrupt_memory()
#else
#define SOS_DEBUG 1
void sos_debug_root_write(const char *buffer, int nbyte);
// int sos_debug_user_printf(const char * format, ...);
// int sos_debug_root_printf(const char * format, ...);
int sos_debug_printf(const char *format, ...);
int sos_debug_vprintf(const char *format, va_list args);

#define sos_debug_root_printf sos_debug_printf
#define sos_debug_user_printf sos_debug_printf

extern int malloc_is_memory_corrupt(struct _reent *reent);
#define sos_debug_trace_corrupt_memory()                                                 \
  do {                                                                                   \
    sos_debug_printf("%s():%d Memory ", __FUNCTION__, __LINE__);                         \
    if (malloc_is_memory_corrupt(0)) {                                                   \
      sos_debug_printf("Corrupt\n", __FUNCTION__, __LINE__);                             \
    } else {                                                                             \
      sos_debug_printf("OK\n", __FUNCTION__, __LINE__);                                  \
    }                                                                                    \
  } while (0)

void sos_debug_log_info(u32 o_flags, const char *format, ...);
void sos_debug_log_warning(u32 o_flags, const char *format, ...);
void sos_debug_log_error(u32 o_flags, const char *format, ...);
void sos_debug_log_fatal(u32 o_flags, const char *format, ...);

#endif

#else

#define sos_debug_printf(format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_info(o_flags, format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_warning(o_flags, format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_error(o_flags, format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_fatal(o_flags, format, ...) printf(format, __VA_ARGS__)

#endif

#define SOS_DEBUG_LINE_TRACE() sos_debug_printf("%s():%d\n", __FUNCTION__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H_ */
