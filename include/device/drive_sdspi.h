// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_SDSPI_H_
#define DEVICE_SDSPI_H_

#include "sos/dev/drive.h"
#include "sos/dev/spi.h"
#include "sos/fs/devfs.h"

#include "mcu/spi.h"

typedef struct {
  const char *buf;
  int *nbyte;
  int count;
  int timeout;
  uint8_t cmd[16];
  devfs_async_t op;
  mcu_event_handler_t handler;
  u32 flags;
} drive_sdspi_state_t;

typedef struct {
  devfs_device_t device;
  mcu_pin_t cs;
} drive_sdspi_config_t;

typedef struct MCU_PACK {
  uint16_t r2;
} drive_sdspi_status_t;

int drive_sdspi_open(const devfs_handle_t *handle);
int drive_sdspi_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_sdspi_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_sdspi_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_sdspi_close(const devfs_handle_t *handle);

int drive_sdspi_dma_open(const devfs_handle_t *handle);
int drive_sdspi_dma_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_sdspi_dma_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_sdspi_dma_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_sdspi_dma_close(const devfs_handle_t *handle);

int drive_sdssp_open(const devfs_handle_t *handle);
int drive_sdssp_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_sdssp_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_sdssp_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_sdssp_close(const devfs_handle_t *handle);

#endif /* DEVICE_SDSPI_H_ */
