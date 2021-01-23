// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_MMC_H_
#define DEVICE_MMC_H_

#include "sos/dev/drive.h"
#include "sos/dev/mmc.h"
#include "sos/fs/devfs.h"

typedef struct {
  const char *buf;
  int *nbyte;
  int count;
  int timeout;
  uint8_t cmd[16];
  devfs_async_t op;
  mcu_event_handler_t handler;
  u32 flags;
} drive_mmc_state_t;

typedef struct {
  devfs_device_t device;
} drive_mmc_config_t;

int drive_mmc_open(const devfs_handle_t *handle);
int drive_mmc_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_mmc_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_mmc_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_mmc_close(const devfs_handle_t *handle);

int drive_mmc_dma_open(const devfs_handle_t *handle);
int drive_mmc_dma_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_mmc_dma_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_mmc_dma_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_mmc_dma_close(const devfs_handle_t *handle);

#endif /* DEVICE_MMC_H_ */
