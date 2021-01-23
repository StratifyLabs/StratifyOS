// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>

#include "config.h"
#include "cortexm/mpu.h"
#include "device/sys.h"
#include "sos/debug.h"
#include "scheduler/scheduler_local.h"
#include "sos/dev/bootloader.h"
#include "sos/dev/sys.h"

#include "device/sys.h"
#include "signal/sig_local.h"
#include "symbols.h"

static int read_task(sys_taskattr_t *task);
static int sys_setattr(const devfs_handle_t *handle, void *ctl);

int sys_open(const devfs_handle_t *handle) { return 0; }

int sys_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  sys_id_t *id = ctl;
  sys_info_t *info = ctl;
  sys_killattr_t *killattr = ctl;
  int i;

  switch (request) {
  case I_SYS_GETINFO:
    memset(info, 0, sizeof(sys_info_t));
    strncpy(info->kernel_version, VERSION, 7);
    strncpy(
      info->sys_version, sos_config.sys.version,
      sizeof(sos_config.sys.version - 1));
    strncpy(info->arch, ARCH, 15);
    info->security = 0;
    info->signature = symbols_table[0];
    info->cpu_freq = sos_config.clock.frequency;
    info->sys_mem_size = sos_config.sys.memory_size;
    info->o_flags = sos_config.sys.flags;
    info->o_mcu_board_config_flags = 0;
    strncpy(info->stdin_name, sos_config.fs.stdin_dev, DEVFS_NAME_MAX);
    strncpy(info->stdout_name, sos_config.fs.stdout_dev, DEVFS_NAME_MAX);
    strncpy(info->name, sos_config.sys.name, APPFS_NAME_MAX);
    strncpy(info->trace_name, sos_config.fs.trace_dev, DEVFS_NAME_MAX);
    strncpy(info->id, sos_config.sys.id, APPFS_ID_MAX);
    strncpy(info->team_id, sos_config.sys.team_id, APPFS_ID_MAX);
    if (sos_config.sys.git_hash) {
      strncpy(info->bsp_git_hash, sos_config.sys.git_hash, 15);
    }
    if (sos_config.sys.mcu_git_hash) {
      strncpy(info->mcu_git_hash, sos_config.sys.mcu_git_hash, 15);
    }
    strncpy(info->sos_git_hash, SOS_GIT_HASH, 15);
    if (sos_config.sys.get_serial_number) {
      sos_config.sys.get_serial_number(&(info->serial));
    }
    info->hardware_id =
      *(((u32 *)cortexm_get_vector_table_addr())
        + BOOTLOADER_HARDWARE_ID_OFFSET / sizeof(u32));
    info->path_max = PATH_MAX;
    info->arg_max = ARG_MAX;
    return 0;

  case I_SYS_GETTASK:
    return read_task(ctl);

  case I_SYS_GETID:
    memcpy(id->id, sos_config.sys.id, PATH_MAX - 1);
    return 0;

  case I_SYS_KILL:
    for (i = 1; i < task_get_total(); i++) {
      if (
        (task_get_pid(i) == killattr->id) && task_enabled(i)
        && !task_thread_asserted(i)) {
        int result = signal_root_send(
          task_get_current(), i, killattr->si_signo, killattr->si_sigcode,
          killattr->si_sigvalue, 1);
        return result;
      }
    }
    return SYSFS_SET_RETURN(EINVAL);

  case I_SYS_PTHREADKILL:
    return signal_root_send(
      task_get_current(), killattr->id, killattr->si_signo, killattr->si_sigcode,
      killattr->si_sigvalue, 1);
  case I_SYS_GETBOARDCONFIG:

    memcpy(ctl, &sos_config, sizeof(sos_config));
    return 0;

#if NOT_BUILDING
  case I_SYS_GETMCUBOARDCONFIG: {
    mcu_board_config_t *config = ctl;
    memcpy(config, &mcu_board_config, sizeof(mcu_board_config));
    // dont' provide any security info to non-root callers
    if (scheduler_authenticated_asserted(task_get_current()) == 0) {
      config->secret_key_address = (void *)-1;
      config->secret_key_size = 0;
    }
    return 0;
  }
#endif

  case I_SYS_SETATTR:
    return sys_setattr(handle, ctl);

  case I_SYS_GETSECRETKEY: {
    sys_secret_key_t *key = ctl;
    memset(key, 0, sizeof(sys_secret_key_t));
    if (scheduler_authenticated_asserted(task_get_current())) {
      u32 size = sos_config.sys.secret_key_size > sizeof(sys_secret_key_t)
                   ? sizeof(sys_secret_key_t)
                   : sos_config.sys.secret_key_size;
      memcpy(key->data, sos_config.sys.secret_key_address - 1, size);
      return 0;
    }
    return SYSFS_SET_RETURN(EPERM);
  }

  case I_SYS_ISAUTHENTICATED:
    return scheduler_authenticated_asserted(task_get_current()) != 0;

  case I_SYS_DEAUTHENTICATE:
    if (scheduler_authenticated_asserted(task_get_current())) {
      scheduler_root_deassert_authenticated(task_get_current());
      return 0;
    }
    return SYSFS_SET_RETURN(EPERM);

  default:
    break;
  }
  return SYSFS_SET_RETURN(EINVAL);
}

int sys_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int sys_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int sys_close(const devfs_handle_t *handle) { return 0; }

int read_task(sys_taskattr_t *task) {
  int ret;
  if (task->tid < task_get_total()) {
    if (task_enabled(task->tid)) {
      task->is_enabled = 1;
      task->pid = task_get_pid(task->tid);
      task->timer = task_root_gettime(task->tid);
      task->mem_loc = (u32)sos_sched_table[task->tid].attr.stackaddr;
      task->mem_size = sos_sched_table[task->tid].attr.stacksize;
      task->stack_ptr = (u32)sos_task_table[task->tid].sp;
      task->prio = task_get_priority(task->tid);
      task->prio_ceiling = sos_sched_table[task->tid].attr.schedparam.sched_priority;
      task->is_active = (task_active_asserted(task->tid) != 0)
                        | ((task_stopped_asserted(task->tid != 0) << 1));
      task->is_thread = task_thread_asserted(task->tid);

      strncpy(
        task->name,
        ((struct _reent *)sos_task_table[task->tid].global_reent)
          ->procmem_base->proc_name,
        LINK_NAME_MAX);

#if 1
      task->malloc_loc = scheduler_calculate_heap_end(task->tid);
#else
      if (!task->is_thread && (sos_task_table[task->tid].reent != NULL)) {
        task->malloc_loc =
          (u32)
          & (((struct _reent *)sos_task_table[task->tid].reent)->procmem_base->base)
              + ((struct _reent *)sos_task_table[task->tid].reent)->procmem_base->size;

      } else {
        task->malloc_loc = 0;
      }
#endif

      ret = 1;

    } else {
      task->is_enabled = 0;
      ret = 0;
    }
  } else {
    // Stratify Link freezes up if this doesn't return -1 -- needs to be fixed
    ret = SYSFS_SET_RETURN_WITH_VALUE(ESRCH, 1);
  }

  return ret;
}

int sys_setattr(const devfs_handle_t *handle, void *ctl) {
  const sys_attr_t *attr = ctl;

  if (attr == 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  u32 o_flags = attr->o_flags;

  if (o_flags & SYS_FLAG_SET_MEMORY_REGION) {

    if (attr->region == TASK_APPLICATION_DATA_USER_REGION) {

      int type = MPU_MEMORY_SRAM;
      if (o_flags & SYS_FLAG_IS_FLASH) {
        type = MPU_MEMORY_FLASH;

      } else if (o_flags & SYS_FLAG_IS_EXTERNAL) {
        type = MPU_MEMORY_EXTERNAL_SRAM;
      }

      int access = MPU_ACCESS_PRW;
      if ((o_flags & SYS_FLAG_IS_READ_ALLOWED) && (o_flags & SYS_FLAG_IS_WRITE_ALLOWED)) {
        access = MPU_ACCESS_PRW_URW;
      } else if (o_flags & SYS_FLAG_IS_READ_ALLOWED) {
        access = MPU_ACCESS_PRW_UR;
      }

      int result = mpu_enable_region(
        attr->region, (void *)attr->address, attr->size, access, type, 0);

      if (result < 0) {
        sos_debug_log_error(
          SOS_DEBUG_SYS, "Failed to enable memory region 0x%lX to 0x%lX (%d)",
          attr->address, attr->size, result);
        return result;
      }

    } else {
      return SYSFS_SET_RETURN(EINVAL);
    }
  }
  return 0;
}
