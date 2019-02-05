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

#ifndef DEVICE_SDSPI_H_
#define DEVICE_SDSPI_H_

#include "sos/fs/devfs.h"
#include "sos/dev/drive.h"
#include "mcu/spi.h"

typedef struct {
	const char * buf;
	int * nbyte;
	int count;
	int timeout;
	uint8_t cmd[16];
	devfs_async_t op;
	mcu_event_handler_t handler;
	u32 flags;
} drive_sdspi_state_t;

typedef struct {
	mcu_pin_t cs;
	u32 spi_config_size;
	spi_config_t spi;
	//some implementatations may add driver specific data here which is encompassed in spi_config_size
} drive_sdspi_config_t;

typedef struct MCU_PACK {
	uint16_t r2;
} drive_sdspi_status_t;

int drive_sdspi_open(const devfs_handle_t * handle);
int drive_sdspi_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int drive_sdspi_read(const devfs_handle_t * handle, devfs_async_t * rop);
int drive_sdspi_write(const devfs_handle_t * handle, devfs_async_t * wop);
int drive_sdspi_close(const devfs_handle_t * handle);

int drive_sdspi_dma_open(const devfs_handle_t * handle);
int drive_sdspi_dma_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int drive_sdspi_dma_read(const devfs_handle_t * handle, devfs_async_t * rop);
int drive_sdspi_dma_write(const devfs_handle_t * handle, devfs_async_t * wop);
int drive_sdspi_dma_close(const devfs_handle_t * handle);

int drive_sdssp_open(const devfs_handle_t * handle);
int drive_sdssp_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int drive_sdssp_read(const devfs_handle_t * handle, devfs_async_t * rop);
int drive_sdssp_write(const devfs_handle_t * handle, devfs_async_t * wop);
int drive_sdssp_close(const devfs_handle_t * handle);



#endif /* DEVICE_SDSPI_H_ */
