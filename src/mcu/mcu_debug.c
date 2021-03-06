// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#if !defined __link

#include <stdarg.h>

#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "mcu/mcu.h"
#include "mcu/uart.h"
#include "sos/dev/uart.h"
#include "sos/sos.h"

#if MCU_DEBUG

static const char *const flag_names[32] = {"SYS", // 0
                                           "SYS",
                                           "SYS", // 2
                                           "CM",
                                           "DEV", // 4
                                           "AIO",     "CRT",    "DIR",       "MALLOC",
                                           "MQ",
                                           "PROCESS", // 10
                                           "PTHREAD", "SCHED",  "SCHEDULER", "SEM",
                                           "SIGNAL", // 15
                                           "FS",      "SOCKET", "TIME",      "APPFS",
                                           "LINK", // 20
                                           "UNISTD",  "USB",    "DEVFS",     "SGFX",
                                           "SON", // 25
                                           "USER0",   "USER1",  "USER2",
                                           "USER3", // 29
                                           "USER4",   "USER5"};

typedef struct {
  char buffer[256];
  int len;
} sos_debug_buffer_t;

static void
sos_debug_vlog(u32 o_flags, const char *intro, const char *format, va_list args);
static void sos_debug_svcall_write_uart(void *args);

void sos_debug_root_write_uart(const char *buffer, int nbyte) {
  int i;

  for (i = 0; i < nbyte; i++) {
    sos_config.debug.write(buffer + i, 1);
  }
}

void sos_debug_svcall_write_uart(void *args) {
  CORTEXM_SVCALL_ENTER();
  sos_debug_buffer_t *p = args;
  sos_debug_root_write_uart(p->buffer, p->len);
}

#if 0
int sos_debug_user_printf(const char * format, ...){
	sos_debug_buffer_t svcall_args;
	va_list args;
	va_start (args, format);
	svcall_args.buffer[255] = 0;
	svcall_args.len = vsnprintf(svcall_args.buffer, 255, format, args);
	cortexm_svcall(sos_debug_root_write_uart_svcall, &svcall_args);
	return svcall_args.len;
}

int sos_debug_root_printf(const char * format, ...){
	int ret = 0;
	char buffer[256];
	buffer[255] = 0;
	va_list args;
	va_start (args, format);
	ret = vsnprintf(buffer, 255, format, args);
	sos_debug_root_write_uart(buffer, ret);
	return ret;
}
#endif

int sos_debug_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  return sos_debug_vprintf(format, args);
}

int sos_debug_vprintf(const char *format, va_list args) {
  sos_debug_buffer_t svcall_args;
  svcall_args.buffer[255] = 0;
  svcall_args.len = vsnprintf(svcall_args.buffer, 255, format, args);
  if (cortexm_is_root_mode()) {
    sos_debug_root_write_uart(svcall_args.buffer, svcall_args.len);
  } else {
    cortexm_svcall(sos_debug_svcall_write_uart, &svcall_args);
  }
  return svcall_args.len;
}

void sos_debug_vlog(u32 o_flags, const char *intro, const char *format, va_list args) {
  if ((sos_config.debug.flags & 0x03) >= (o_flags & 3)) { // check the level
    if ((sos_config.debug.flags & o_flags) & ~0x03) {     // check the subsystem
      u32 first_flag = __builtin_ctz(o_flags & ~0x03);
      sos_debug_printf("%s:%s:", intro, flag_names[first_flag]);
      sos_debug_vprintf(format, args);
      sos_debug_printf("\n");
    }
  }
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
