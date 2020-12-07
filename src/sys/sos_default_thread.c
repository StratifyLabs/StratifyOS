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

#include "config.h"
#include "sos/debug.h"
#include "mcu/pio.h"
#include "sos/fs/sysfs.h"
#include "sos/link.h"
#include "sos/sos.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

extern void *link_update(void *args);

static void init_fs();
static void start_filesystem(void);
static int startup_fs();

static void svcall_core_getinfo(void *args) MCU_ROOT_EXEC_CODE;

void svcall_core_getinfo(void *args) {
  CORTEXM_SVCALL_ENTER();
  mcu_core_getinfo(0, args);
}

void check_reset_source(void) {
  core_info_t info;
  cortexm_svcall(svcall_core_getinfo, &info);
  if (info.o_flags & CORE_FLAG_IS_RESET_SOFTWARE) {
    SOS_TRACE_MESSAGE("soft reset");
  } else if (info.o_flags & CORE_FLAG_IS_RESET_POR) {
    SOS_TRACE_MESSAGE("por reset");
  } else if (info.o_flags & CORE_FLAG_IS_RESET_EXTERNAL) {
    SOS_TRACE_MESSAGE("ext reset");
  } else if (info.o_flags & CORE_FLAG_IS_RESET_WDT) {
    SOS_TRACE_MESSAGE("wdt reset");
  } else if (info.o_flags & CORE_FLAG_IS_RESET_BOR) {
    SOS_TRACE_MESSAGE("bor reset");
  } else if (info.o_flags & CORE_FLAG_IS_RESET_SYSTEM) {
    SOS_TRACE_MESSAGE("sys reset");
  }
  sos_handle_event(SOS_EVENT_RESET_SOURCE, &info);
}

void start_filesystem(void) {
  u32 started;
  started = startup_fs();
  sos_debug_log_info(SOS_DEBUG_SYS, "Started Filesystem Apps %ld", started);
  sos_handle_event(SOS_EVENT_FILESYSTEM_INITIALIZED, &started);
}

void *sos_default_thread(void *arg) {
  sos_debug_log_info(SOS_DEBUG_SYS, "Enter default thread-");

  check_reset_source();

  // Initialize the file systems
  init_fs();
  SOS_DEBUG_LINE_TRACE();
  start_filesystem();
  SOS_DEBUG_LINE_TRACE();

  sos_handle_event(SOS_EVENT_START_LINK, 0);
  link_update(arg); // Run the link update thread--never returns
  return NULL;
}

void init_fs() {
  int i;
  i = 0;
  const sysfs_t *sysfs_list = sos_config.fs.rootfs_list;

  while (sysfs_isterminator(sysfs_list + i) == 0) {
    SOS_TRACE_MESSAGE(sysfs_list[i].mount_path);
    sos_debug_log_info(SOS_DEBUG_SYS, "init %s", sysfs_list[i].mount_path);
    int result;
    if ((result = sysfs_list[i].mount(sysfs_list[i].config)) < 0) {
      sos_debug_log_error(
        SOS_DEBUG_SYS, "failed to init %d, %d", SYSFS_GET_RETURN(result),
        SYSFS_GET_RETURN_ERRNO(result));
      sos_handle_event(SOS_EVENT_FATAL, (void *)"init_fs");
    }
    i++;
  }
}

int startup_fs() {
  int i;
  i = 0;
  int started;
  started = 0;
  const sysfs_t *sysfs_list = sos_config.fs.rootfs_list;

  while (sysfs_isterminator(sysfs_list + i) == 0) {
    int ret = sysfs_list[i].startup(sysfs_list[i].config);
    if (ret > 0) {
      started += ret;
    }
    i++;
  }
  // the number of processes that have been started
  return started;
}
