/* Copyright 2011-2017 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef DEVICE_SDIO_H_
#define DEVICE_SDIO_H_

#include "mcu/sdio.h"
#include "sos/dev/drive.h"
#include "sos/fs/devfs.h"

typedef struct {
  u32 flags;
} drive_sdio_state_t;

typedef struct {
  sdio_config_t sdio;
} drive_sdio_config_t;

int drive_sdio_open(const devfs_handle_t *handle);
int drive_sdio_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_sdio_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_sdio_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_sdio_close(const devfs_handle_t *handle);

int drive_sdio_dma_open(const devfs_handle_t *handle);
int drive_sdio_dma_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int drive_sdio_dma_read(const devfs_handle_t *handle, devfs_async_t *rop);
int drive_sdio_dma_write(const devfs_handle_t *handle, devfs_async_t *wop);
int drive_sdio_dma_close(const devfs_handle_t *handle);

#endif /* DEVICE_SD_SPI_H_ */
