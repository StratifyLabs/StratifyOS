// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_SDIO_H_
#define DEVICE_SDIO_H_

#include "sos/dev/drive.h"
#include "sos/dev/sdio.h"
#include "sos/fs/devfs.h"

typedef struct {
  u32 flags;
} drive_sdio_state_t;

typedef struct {
  devfs_device_t device;
} drive_sdio_config_t;

int drive_sdio_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int drive_sdio_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int drive_sdio_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_EXEC_CODE;
int drive_sdio_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_EXEC_CODE;
int drive_sdio_close(const devfs_handle_t *handle);

int drive_sdio_dma_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int drive_sdio_dma_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int drive_sdio_dma_read(const devfs_handle_t *handle, devfs_async_t *rop)
  MCU_ROOT_EXEC_CODE;
int drive_sdio_dma_write(const devfs_handle_t *handle, devfs_async_t *wop)
  MCU_ROOT_EXEC_CODE;
int drive_sdio_dma_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

#endif /* DEVICE_SD_SPI_H_ */
