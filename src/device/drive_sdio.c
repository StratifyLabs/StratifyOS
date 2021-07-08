// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <unistd.h>

#include "sos/sos.h"

#include "cortexm/cortexm.h"
#include "device/drive_sdio.h"
#include "sos/debug.h"

#include "cortexm/task.h"

int drive_sdio_open(const devfs_handle_t *handle) {
  DEVFS_DRIVER_DECLARE_CONFIG(drive_sdio);
  return config->device.driver.open(&config->device.handle);
}

static int handle_misaligned_read_complete(void *context, const mcu_event_t *event) {
  MCU_UNUSED_ARGUMENT(event);
  const devfs_handle_t *handle = context;
  DEVFS_DRIVER_DECLARE_CONFIG_STATE(drive_sdio);

  // copy the data to the original buffer
  memcpy(
    (u8 *)state->async_buffer + state->bytes_read, config->dma_read_buffer,
    state->async->nbyte);

  state->bytes_read += state->async->nbyte;

  int result = 0;

  if (state->bytes_read < state->async_nbyte) {
    // read another block
    result = config->device.driver.read(&config->device.handle, state->async);
  } else {
    // finalize the transaction
    result = state->async_nbyte;
  }

  if (result != 0) {
    // restore the async object
    state->async->buf = state->async_buffer;
    state->async->nbyte = state->async_nbyte;
    state->async->handler = state->async_handler;

    // mark as not busy
    state->async_handler.callback = 0;
    state->async->result = result;

    if (result < 0) {
      devfs_execute_event_handler(
        &state->async->handler, MCU_EVENT_FLAG_ERROR | MCU_EVENT_FLAG_CANCELED, 0);
    } else {
      devfs_execute_event_handler(&state->async->handler, MCU_EVENT_FLAG_DATA_READY, 0);
    }

    return 0;
  }
}

int drive_sdio_read(const devfs_handle_t *handle, devfs_async_t *async) {
  DEVFS_DRIVER_DECLARE_CONFIG_STATE(drive_sdio);
  if (config->dma_read_buffer) {
    if (state->async_handler.callback != 0) {
      return SYSFS_SET_RETURN(EBUSY);
    }

    if ((u32)async->buf % 32) {
      // can't have misaligned reads -- need to use the dedicated buffer
      // save the originals
      state->async_buffer = async->buf;
      state->async_handler = async->handler;
      state->async_nbyte = async->nbyte;
      state->bytes_read = 0;
      state->async = async;

      // re-route the async object to use the aligned DMA buffer
      async->buf = config->dma_read_buffer;
      async->handler.callback = handle_misaligned_read_complete;
      async->handler.context = (void *)handle;

      // adjust the size accordingly
      if (async->nbyte > config->dma_read_buffer_size) {
        async->nbyte = config->dma_read_buffer_size;
      }
    }
  }
  return config->device.driver.read(&config->device.handle, async);
}

int drive_sdio_write(const devfs_handle_t *handle, devfs_async_t *async) {
  DEVFS_DRIVER_DECLARE_CONFIG(drive_sdio);
  return config->device.driver.write(&config->device.handle, async);
}

int drive_sdio_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  DEVFS_DRIVER_DECLARE_CONFIG(drive_sdio);
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
  DEVFS_DRIVER_DECLARE_CONFIG(drive_sdio);
  return config->device.driver.close(&config->device.handle);
}

/*! @} */
