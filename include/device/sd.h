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


#ifndef DEVICE_SD_H_
#define DEVICE_SD_H_

#include "sos/dev/sd.h"
#include "sos/dev/spi.h"
#include "sos/fs/devfs.h"

typedef struct {
	const char * buf;
	int * nbyte;
	int count;
	int timeout;
	uint8_t cmd[16];
	devfs_async_t op;
	mcu_event_handler_t handler;
	u32 flags;
} sdspi_state_t;

typedef struct {
	spi_attr_t attr;
	mcu_pin_t cs;
	mcu_pin_t hold /*! \brief Hold Pin */;
	mcu_pin_t wp /*! \brief Write protect pin */;
	mcu_pin_t miso /*! \brief The write complete pin */;
	uint32_t size /*! \brief The size of the memory on the device */;
} sdspi_config_t;


typedef struct MCU_PACK {
	uint16_t r2;
} sdspi_status_t;

int sdspi_open(const devfs_handle_t * handle);
int sdspi_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int sdspi_read(const devfs_handle_t * handle, devfs_async_t * rop);
int sdspi_write(const devfs_handle_t * handle, devfs_async_t * wop);
int sdspi_close(const devfs_handle_t * handle);

int sdssp_open(const devfs_handle_t * handle);
int sdssp_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int sdssp_read(const devfs_handle_t * handle, devfs_async_t * rop);
int sdssp_write(const devfs_handle_t * handle, devfs_async_t * wop);
int sdssp_close(const devfs_handle_t * handle);


#endif /* DEVICE_SD_H_ */
