// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "config.h"
#include "device/sys.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "scheduler/scheduler_local.h"
#include "signal/sig_local.h"
#include "symbols.h"
#include <errno.h>

int sys_23_open(const devfs_handle_t *handle) { return sys_open(handle); }

int sys_23_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  sys_23_info_t *sys = ctl;

  switch (request) {
  case I_SYS_GETINFO: // this will be the wrong version
    break;
  case I_SYS_23_GETINFO:
    memset(sys, 0, sizeof(sys_23_info_t));
    strncpy(sys->version, VERSION, 7);
    strncpy(sys->sys_version, sos_board_config.sys.version, 7);
    strncpy(sys->arch, ARCH, 7);
    sys->security = 0;
    sys->signature = symbols_table[0];
    sys->cpu_freq = sos_board_config.clock.frequency;
    sys->sys_mem_size = sos_board_config.sys.memory_size;
    sys->o_flags = sos_board_config.sys.flags;
    strncpy(sys->stdin_name, sos_board_config.fs.stdin_dev, NAME_MAX - 1);
    strncpy(sys->stdout_name, sos_board_config.fs.stdout_dev, NAME_MAX - 1);
    strncpy(sys->name, sos_board_config.sys.name, NAME_MAX - 1);
    mcu_core_getserialno(&(sys->serial));
    return 0;
  default:
    return sys_ioctl(handle, request, ctl);
  }
  errno = EINVAL;
  return -1;
}

int sys_23_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  return sys_read(handle, rop);
}

int sys_23_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  return sys_write(handle, wop);
}

int sys_23_close(const devfs_handle_t *handle) { return sys_close(handle); }
