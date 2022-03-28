// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#if !defined __link

#include <stdarg.h>

#include "cortexm/cortexm.h"
#include "sos/debug.h"
#include "sos/sos.h"

#if SOS_DEBUG

static const char *const flag_names[] = {
  "NA", // 0
  "NA", //1
  "NA", // 2
  "NA", // 3
  "NA", // 4
  "NA", // 5
  "NA", // 6
  "NA", // 7
  "DEV", // 8
  "AIO",//9
  "CRT",//10
  "MALLOC",//11
  "PROCESS", // 12
  "PTHREAD",//13
  "SCHED",//14
  "SCHEDULER",//15
  "SIGNAL", // 16
  "FS",//17
  "SOCKET",//18
  "APPFS",//19
  "LINK", // 20
  "UNISTD",//21
  "DEVFS",//22
  "SYS", // 23
  "USER0",//24
  "USER1",//25
  "USER2", //26
  "USER3", // 27
  "USER4", //28
  "USER5," //29
  "TASK" //30
};

typedef struct {
  char buffer[256];
  int len;
} sos_debug_buffer_t;

static void
sos_debug_vlog(u32 o_flags, const char *intro, const char *format, va_list args);
static void sos_debug_svcall_write(void *args);

void sos_debug_svcall_write(void *args) {
  CORTEXM_SVCALL_ENTER();
  sos_debug_buffer_t *p = args;
  sos_config.debug.write(p->buffer, p->len);
}

int sos_debug_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int is_running = 0;
  const int result = sos_debug_vprintf(format, args);
  return result;
}

int sos_debug_vprintf(const char *format, va_list args) {
  sos_debug_buffer_t svcall_args;
  svcall_args.buffer[255] = 0;
  svcall_args.len = vsniprintf(svcall_args.buffer, 255, format, args);
  if (cortexm_is_root_mode()) {
    sos_config.debug.write(svcall_args.buffer, svcall_args.len);
  } else {
    cortexm_svcall(sos_debug_svcall_write, &svcall_args);
  }
  return svcall_args.len;
}

void sos_debug_vlog(u32 o_flags, const char *intro, const char *format, va_list args) {
  if ((sos_config.debug.flags & 0x07) >= (o_flags & 7)) { // check the level
    if ((sos_config.debug.flags & o_flags) & ~0x07) {     // check the subsystem
      u32 first_flag = __builtin_ctz(o_flags & ~0x07);
      sos_debug_printf("%s:%s:", intro, flag_names[first_flag]);
      sos_debug_vprintf(format, args);
      sos_debug_printf("\n");
    }
  }
}

void sos_debug_vlog_report(u32 o_flags, const char *intro, const char *format, va_list args) {
  if ((sos_config.debug.flags & 0x07) >= (o_flags & 7)) { // check the level
    if ((sos_config.debug.flags & o_flags) & ~0x07) {     // check the subsystem
      sos_debug_printf("%s:", intro);
      sos_debug_vprintf(format, args);
      sos_debug_printf("\n");
    }
  }
}

void sos_debug_log_datum(u32 o_flags, const char *format, ...){
  va_list args;
  va_start(args, format);
  sos_debug_vlog_report(SOS_DEBUG_DATUM | o_flags, "DAT", format, args);
}

void sos_debug_log_event(u32 o_flags, const char *format, ...){
  va_list args;
  va_start(args, format);
  sos_debug_vlog_report(SOS_DEBUG_EVENT | o_flags, "EV", format, args);
}

void sos_debug_log_directive(u32 o_flags, const char *format, ...){
  va_list args;
  va_start(args, format);
  sos_debug_vlog_report(SOS_DEBUG_DIRECTIVE | o_flags, "DIR", format, args);
}

void sos_debug_log_message(u32 o_flags, const char *format, ...){
  va_list args;
  va_start(args, format);
  sos_debug_vlog_report(SOS_DEBUG_MESSAGE | o_flags, "MSG", format, args);
}

void sos_debug_log_info(u32 o_flags, const char *format, ...) {
  va_list args;
  va_start(args, format);
  sos_debug_vlog(SOS_DEBUG_INFO | o_flags, "INFO", format, args);
}

void sos_debug_log_warning(u32 o_flags, const char *format, ...) {
  va_list args;
  va_start(args, format);
  sos_debug_vlog(SOS_DEBUG_WARNING | o_flags, "WARN", format, args);
}

void sos_debug_log_error(u32 o_flags, const char *format, ...) {
  va_list args;
  va_start(args, format);
  sos_debug_vlog(SOS_DEBUG_ERROR | o_flags, "ERR", format, args);
}

void sos_debug_log_fatal(u32 o_flags, const char *format, ...) {
  va_list args;
  va_start(args, format);
  sos_debug_vlog(o_flags, "FATAL", format, args);
}

#endif

#endif
