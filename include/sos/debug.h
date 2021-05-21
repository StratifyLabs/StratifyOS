// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef _SOS_DEBUG_H_
#define _SOS_DEBUG_H_

#include <sdk/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cortexm/cortexm.h"

#ifdef __cplusplus
extern "C" {
#endif

enum sos_debug_flags {
  SOS_DEBUG_ERROR = 1,
  SOS_DEBUG_WARNING = 2,
  SOS_DEBUG_INFO = 3,
  SOS_DEBUG_DATUM = 4,
  SOS_DEBUG_EVENT = 5,
  SOS_DEBUG_DIRECTIVE = 6,
  SOS_DEBUG_MESSAGE = 7,
  SOS_DEBUG_DEVICE = (1 << 8),
  SOS_DEBUG_AIO = (1 << 9),
  SOS_DEBUG_CRT = (1 << 10),
  SOS_DEBUG_MALLOC = (1 << 11),
  SOS_DEBUG_PROCESS = (1 << 12),
  SOS_DEBUG_PTHREAD = (1 << 13),
  SOS_DEBUG_SCHED = (1 << 14),
  SOS_DEBUG_SCHEDULER = (1 << 15),
  SOS_DEBUG_SIGNAL = (1 << 16),
  SOS_DEBUG_FILESYSTEM = (1 << 17),
  SOS_DEBUG_SOCKET = (1 << 18),
  SOS_DEBUG_APPFS = (1 << 19),
  SOS_DEBUG_LINK = (1 << 20),
  SOS_DEBUG_UNISTD = (1 << 21),
  SOS_DEBUG_DEVFS = (1 << 22),
  SOS_DEBUG_SYS = (1 << 23),
  SOS_DEBUG_USER0 = (1 << 24),
  SOS_DEBUG_USER1 = (1 << 25),
  SOS_DEBUG_USER2 = (1 << 26),
  SOS_DEBUG_USER3 = (1 << 27),
  SOS_DEBUG_USER4 = (1 << 28),
  SOS_DEBUG_USER5 = (1 << 29),
  SOS_DEBUG_TASK = (1 << 30)
};

#ifndef __link

#if defined __debug
#define ___debug
#endif

#if !defined ___debug
#define sos_debug_user_printf(...)
#define sos_debug_printf(...)
#define sos_debug_root_printf(...)
#define sos_debug_log_datum(o_flags, format, ...)
#define sos_debug_log_event(o_flags, format, ...)
#define sos_debug_log_directive(o_flags, format, ...)
#define sos_debug_log_message(o_flags, format, ...)
#define sos_debug_log_info(o_flags, format, ...)
#define sos_debug_log_warning(o_flags, format, ...)
#define sos_debug_log_error(o_flags, format, ...)
#define sos_debug_log_fatal(o_flags, format, ...)
#define sos_debug_trace_corrupt_memory()
#define SOS_DEBUG_ENTER_CYCLE_SCOPE(name_value)
#define SOS_DEBUG_EXIT_CYCLE_SCOPE(flags, name_value)
#define SOS_DEBUG_ENTER_CYCLE_SCOPE_AVERAGE()
#define SOS_DEBUG_EXIT_CYCLE_SCOPE_AVERAGE(flag_value, name_value, count_value)
#define SOS_DEBUG_ENTER_TIMER_SCOPE(name_value)
#define SOS_DEBUG_EXIT_TIMER_SCOPE(flags, name_value)
#define SOS_DEBUG_ENTER_TIMER_SCOPE_AVERAGE(name_value)
#define SOS_DEBUG_EXIT_TIMER_SCOPE_AVERAGE(flag_value, name_value, count_value)
#else
#define SOS_DEBUG 1
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

void sos_debug_log_datum(u32 o_flags, const char *format, ...);
void sos_debug_log_event(u32 o_flags, const char *format, ...);
void sos_debug_log_directive(u32 o_flags, const char *format, ...);
void sos_debug_log_message(u32 o_flags, const char *format, ...);
void sos_debug_log_info(u32 o_flags, const char *format, ...);
void sos_debug_log_warning(u32 o_flags, const char *format, ...);
void sos_debug_log_error(u32 o_flags, const char *format, ...);
void sos_debug_log_fatal(u32 o_flags, const char *format, ...);

#define SOS_DEBUG_ENTER_CYCLE_SCOPE() cortexm_enter_cycle_scope()

#define SOS_DEBUG_EXIT_CYCLE_SCOPE(flags, name_value)                                    \
  sos_debug_log_datum(                                                                   \
    flags, MCU_STRINGIFY(name_value) "_cycles:%ld", cortexm_exit_cycle_scope())

#define SOS_DEBUG_ENTER_CYCLE_SCOPE_AVERAGE()                                            \
  static u32 sos_debug_cycle_count = 0;                                                  \
  static u32 sos_debug_cycle_sum = 0;                                                    \
  cortexm_enter_cycle_scope()

#define SOS_DEBUG_EXIT_CYCLE_SCOPE_AVERAGE(flag_value, name_value, count_value)          \
  do {                                                                                   \
    sos_debug_cycle_sum += cortexm_exit_cycle_scope();                                   \
    if (sos_debug_cycle_count++ == count_value) {                                        \
      sos_debug_log_datum(                                                               \
        flag_value, MCU_STRINGIFY(name_value) ":%ld",                                    \
        sos_debug_cycle_sum / count_value);                                              \
      sos_debug_cycle_count = 0;                                                         \
      sos_debug_cycle_sum = 0;                                                           \
    }                                                                                    \
  } while (0)

#define SOS_DEBUG_ENTER_TIMER_SCOPE(name_value)                                          \
  u64 sos_debug_timer_scope_##name_value = sos_realtime()

#define SOS_DEBUG_EXIT_TIMER_SCOPE(flags, name_value)                                    \
  sos_debug_log_datum(                                                                   \
    flags, MCU_STRINGIFY(name_value) "_us:%ld",                                          \
    (u32)(sos_realtime() - sos_debug_timer_scope_##name_value))

#define SOS_DEBUG_ENTER_TIMER_SCOPE_AVERAGE(name_value)                                  \
  static u32 name_value##_count = 0;                                                     \
  static u32 name_value##_sum = 0;                                                       \
  u64 sos_debug_timer_scope_##name_value = sos_realtime()

#define SOS_DEBUG_EXIT_TIMER_SCOPE_AVERAGE(flag_value, name_value, count_value)          \
  do {                                                                                   \
    name_value##_sum += sos_realtime() - sos_debug_timer_scope_##name_value;             \
    if (name_value##_count++ == count_value) {                                           \
      name_value##_count = 0;                                                            \
      name_value##_sum = 0;                                                              \
      sos_debug_log_datum(                                                               \
        flag_value, MCU_STRINGIFY(name_value) ":%ld", name_value##_sum / count_value);   \
    }                                                                                    \
  } while (0)

#endif

#else

#define sos_debug_printf(format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_datum(o_flags, format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_event(o_flags, format, ...) printf(format, __VA_ARGS__)
#define sos_debug_log_directive(o_flags, format, ...) printf(format, __VA_ARGS__)
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
