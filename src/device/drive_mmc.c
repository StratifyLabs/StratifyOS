// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "sos/sos.h"

#include "cortexm/cortexm.h"
#include "device/drive_mmc.h"
#include "sos/debug.h"
#include <errno.h>
#include <unistd.h>

#include "cortexm/task.h"
#include "mcu/core.h"

int drive_mmc_open(const devfs_handle_t *handle) {
  const drive_mmc_config_t *config = handle->config;
  return config->device.driver.open(&config->device.handle);
}

int drive_mmc_read(const devfs_handle_t *handle, devfs_async_t *async) {
  const drive_mmc_config_t *config = handle->config;
  return config->device.driver.read(&config->device.handle, async);
}

int drive_mmc_write(const devfs_handle_t *handle, devfs_async_t *async) {
  const drive_mmc_config_t *config = handle->config;
  return config->device.driver.write(&config->device.handle, async);
}

int drive_mmc_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  const drive_mmc_config_t *config = handle->config;
  drive_info_t *info = ctl;
  drive_attr_t *attr = ctl;
  u32 o_flags;
  mmc_info_t mmc_info;
  mmc_attr_t mmc_attr;
  int result;

  switch (request) {
  case I_DRIVE_SETATTR:
    o_flags = attr->o_flags;
    if (o_flags & (DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_ERASE_DEVICE)) {

      if (o_flags & DRIVE_FLAG_ERASE_BLOCKS) {
        mmc_attr.o_flags = MMC_FLAG_ERASE_BLOCKS;
        mmc_attr.start = attr->start;
        mmc_attr.end = attr->end;
        result =
          config->device.driver.ioctl(&config->device.handle, I_MMC_SETATTR, &mmc_attr);
        if (result < 0) {
          return result;
        }
        return attr->end - attr->start;
      }
    }

    if (o_flags & DRIVE_FLAG_INIT) {

      // this will init the SD card with the default settings
      return config->device.driver.ioctl(&config->device.handle, I_MMC_SETATTR, NULL);
    }

    if (o_flags & DRIVE_FLAG_RESET) {
      mmc_attr.o_flags = MMC_FLAG_RESET;
      return config->device.driver.ioctl(
        &config->device.handle, I_MMC_SETATTR, &mmc_attr);
    }

    break;

  case I_DRIVE_ISBUSY:
    mmc_attr.o_flags = MMC_FLAG_GET_CARD_STATE;
    result =
      config->device.driver.ioctl(&config->device.handle, I_MMC_SETATTR, &mmc_attr);
    if (result < 0) {
      return result;
    }

    switch (result) {
    default:
      // case MMC_CARD_STATE_NONE:
      // case MMC_CARD_STATE_READY:
      // case MMC_CARD_STATE_DISCONNECTED:
      // case MMC_CARD_STATE_ERROR:
      // case MMC_CARD_STATE_TRANSFER:
      // these states mean the device is ready
      return 0;
    case MMC_CARD_STATE_IDENTIFICATION:
    case MMC_CARD_STATE_STANDBY:
    case MMC_CARD_STATE_SENDING:
    case MMC_CARD_STATE_RECEIVING:
    case MMC_CARD_STATE_PROGRAMMING:
      // these states mean the device is busy temporarily
      return 1;
    }

  case I_DRIVE_GETINFO:
    result =
      config->device.driver.ioctl(&config->device.handle, I_MMC_GETINFO, &mmc_info);
    if (result < 0) {
      return result;
    }

    info->o_flags = DRIVE_FLAG_INIT | DRIVE_FLAG_ERASE_BLOCKS | DRIVE_FLAG_RESET;
    info->o_events = mmc_info.o_events;
    info->addressable_size = mmc_info.block_size;
    info->bitrate = mmc_info.freq;
    info->erase_block_size = mmc_info.block_size;
    info->erase_block_time = 0;
    info->erase_device_time = 0;
    info->num_write_blocks = mmc_info.block_count;
    info->write_block_size = mmc_info.block_size;
    break;

  default:
    return mcu_mmc_ioctl(handle, request, ctl);
  }
  return 0;
}

int drive_mmc_close(const devfs_handle_t *handle) {
  const drive_mmc_config_t *config = handle->config;
  return config->device.driver.close(&config->device.handle);
}

/*! @} */
