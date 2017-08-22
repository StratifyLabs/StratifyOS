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

#ifndef DEVICE_SD_SPI_H_
#define DEVICE_SD_SPI_H_

#include "sos/dev/sd.h"
#include "sos/fs/devfs.h"
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
} sd_spi_state_t;

typedef struct {
	spi_config_t spi;
	mcu_pin_t cs;
} sd_spi_config_t;

typedef struct MCU_PACK {
	uint16_t r2;
} sd_spi_status_t;

int sd_spi_open(const devfs_handle_t * handle);
int sd_spi_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int sd_spi_read(const devfs_handle_t * handle, devfs_async_t * rop);
int sd_spi_write(const devfs_handle_t * handle, devfs_async_t * wop);
int sd_spi_close(const devfs_handle_t * handle);

int sd_ssp_open(const devfs_handle_t * handle);
int sd_ssp_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int sd_ssp_read(const devfs_handle_t * handle, devfs_async_t * rop);
int sd_ssp_write(const devfs_handle_t * handle, devfs_async_t * wop);
int sd_ssp_close(const devfs_handle_t * handle);


#endif /* DEVICE_SD_SPI_H_ */
