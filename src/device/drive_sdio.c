// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <unistd.h>

#include "sos/sos.h"

#include "cortexm/cortexm.h"
#include "device/drive_sdio.h"
#include "sos/debug.h"

#include "cortexm/task.h"

int drive_sdio_open(const devfs_handle_t *handle) {
  const drive_sdio_config_t *config = handle->config;
  return config->device.driver.open(&config->device.handle);
}

int drive_sdio_read(const devfs_handle_t *handle, devfs_async_t *async) {
  const drive_sdio_config_t *config = handle->config;
  return config->device.driver.read(&config->device.handle, async);
}

int drive_sdio_write(const devfs_handle_t *handle, devfs_async_t *async) {
  const drive_sdio_config_t *config = handle->config;
  return config->device.driver.write(&config->device.handle, async);
}

int drive_sdio_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  const drive_sdio_config_t *config = handle->config;
  drive_info_t *info = ctl;
  drive_attr_t *attr = ctl;
  u32 o_flags;
  sdio_info_t sdio_info;
  sdio_attr_t sdio_attr;
  int result;

  switch (request) {
  case I_DRIVE_SETATTR:
    o_flags = attr->o_flags;
    if (o_flags & (DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_ERASE_DEVICE)) {

      if (o_flags & DRIVE_FLAG_ERASE_BLOCKS) {

        sdio_attr.o_flags = SDIO_FLAG_ERASE_BLOCKS;
        sdio_attr.start = attr->start;
        sdio_attr.end = attr->end;
        result =
          config->device.driver.ioctl(&config->device.handle, I_SDIO_SETATTR, &sdio_attr);

        if (result < 0) {
          return result;
        }
        return attr->end - attr->start;
      }
    }

    if (o_flags & DRIVE_FLAG_INIT) {
      // this will init the SD card with the default settings
      return config->device.driver.ioctl(&config->device.handle, I_SDIO_SETATTR, NULL);
    }

    if (o_flags & DRIVE_FLAG_RESET) {
      sdio_attr.o_flags = SDIO_FLAG_RESET;
      return config->device.driver.ioctl(
        &config->device.handle, I_SDIO_SETATTR, &sdio_attr);
    }

    return 0;

  case I_DRIVE_ISBUSY:
    sdio_attr.o_flags = SDIO_FLAG_GET_CARD_STATE;
    result =
      config->device.driver.ioctl(&config->device.handle, I_SDIO_SETATTR, &sdio_attr);

    if (result < 0) {
      return result;
    }

    return (result != SDIO_CARD_STATE_TRANSFER);

  case I_DRIVE_GETINFO:
    result =
      config->device.driver.ioctl(&config->device.handle, I_SDIO_GETINFO, &sdio_info);

    if (result < 0) {
      return result;
    }

    info->o_flags = DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_INIT;
    info->o_events = sdio_info.o_events;
    info->addressable_size = sdio_info.block_size;
    info->bitrate = sdio_info.freq;
    info->erase_block_size = sdio_info.block_size;
    info->erase_block_time = 0;
    info->erase_device_time = 0;
    info->num_write_blocks = sdio_info.block_count;
    info->write_block_size = sdio_info.block_size;
    info->page_program_size = sdio_info.block_size;
    return 0;
  }
  return config->device.driver.ioctl(&config->device.handle, request, ctl);
}

int drive_sdio_close(const devfs_handle_t *handle) {
  const drive_sdio_config_t *config = handle->config;
  return config->device.driver.close(&config->device.handle);
}

/*! @} */
