// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DRIVE_DEVICE_H
#define DRIVE_DEVICE_H

#include "mcu/sdio.h"
#include "sos/dev/drive.h"
#include "sos/fs/devfs.h"

typedef struct {
  u32 flags;
} drive_device_state_t;

typedef struct {
  devfs_device_t device;
} drive_device_config_t;

int drive_device_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int drive_device_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int drive_device_read(const devfs_handle_t *handle, devfs_async_t *rop)
  MCU_ROOT_EXEC_CODE;
int drive_device_write(const devfs_handle_t *handle, devfs_async_t *wop)
  MCU_ROOT_EXEC_CODE;
int drive_device_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

int drive_device_dma_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int drive_device_dma_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int drive_device_dma_read(const devfs_handle_t *handle, devfs_async_t *rop)
  MCU_ROOT_EXEC_CODE;
int drive_device_dma_write(const devfs_handle_t *handle, devfs_async_t *wop)
  MCU_ROOT_EXEC_CODE;
int drive_device_dma_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

#endif // DRIVE_DEVICE_H
