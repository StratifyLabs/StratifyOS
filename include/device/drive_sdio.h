// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_SDIO_H_
#define DEVICE_SDIO_H_

#include "sos/dev/drive.h"
#include "sos/dev/sdio.h"
#include "sos/fs/devfs.h"

typedef struct {
  u32 flags;
  void *async_buffer;
  devfs_async_t *async;
  int bytes_read;
  int async_nbyte;
  mcu_event_handler_t async_handler;
} drive_sdio_state_t;

typedef struct {
  devfs_device_t device;
  void * dma_read_buffer;
  u16 dma_read_buffer_size;
} drive_sdio_config_t;

int drive_sdio_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int drive_sdio_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int drive_sdio_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_EXEC_CODE;
int drive_sdio_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_EXEC_CODE;
int drive_sdio_close(const devfs_handle_t *handle);

#endif /* DEVICE_SD_SPI_H_ */
