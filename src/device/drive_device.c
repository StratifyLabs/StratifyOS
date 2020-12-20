// Copyright 2016-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <unistd.h>

#include "sos/sos.h"

#include "cortexm/cortexm.h"
#include "device/drive_device.h"
#include "sos/debug.h"

#include "cortexm/task.h"
#include "mcu/core.h"

int drive_device_open(const devfs_handle_t *handle) {
  const drive_device_config_t *config = handle->config;
  return config->device.driver.open(&config->device.handle);
}

int drive_device_read(const devfs_handle_t *handle, devfs_async_t *async) {
  const drive_device_config_t *config = handle->config;
  return config->device.driver.read(&config->device.handle, async);
}

int drive_device_write(const devfs_handle_t *handle, devfs_async_t *async) {
  const drive_device_config_t *config = handle->config;
  return config->device.driver.write(&config->device.handle, async);
}

int drive_device_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  const drive_device_config_t *config = handle->config;
  drive_info_t *info = ctl;
  drive_attr_t *attr = ctl;
  u32 o_flags;
  int result;

  switch (request) {
  case I_DRIVE_SETATTR:
    o_flags = attr->o_flags;
    if (o_flags & (DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_ERASE_DEVICE)) {

      if (o_flags & DRIVE_FLAG_ERASE_BLOCKS) {
      }
    }

    if (o_flags & DRIVE_FLAG_INIT) {
    }

    if (o_flags & DRIVE_FLAG_RESET) {
    }

    break;

  case I_DRIVE_ISBUSY:

    return 0;
  case I_DRIVE_GETINFO:

    return 0;
  }
  return config->device.driver.ioctl(&config->device.handle, request, ctl);
}

int drive_device_close(const devfs_handle_t *handle) {
  const drive_device_config_t *config = handle->config;
  return config->device.driver.close(&config->device.handle);
}

/*! @} */
