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

#ifndef DEVICE_CFI_H_
#define DEVICE_CFI_H_

#include "sos/fs/devfs.h"
#include "sos/dev/drive.h"
#include "mcu/spi.h"
#include "mcu/qspi.h"

typedef struct {
	const char * buf;
	int * nbyte;
	int count;
	int timeout;
	uint8_t cmd[16];
    mcu_event_handler_t handler;
	u32 flags;
} drive_cfi_state_t;

typedef struct {
    spi_config_t spi;
} drive_cfi_spi_config_t;

typedef struct {
    qspi_config_t qspi;
} drive_cfi_qspi_config_t;



DEVFS_DRIVER_DECLARTION(cfi_spi);
DEVFS_DRIVER_DECLARTION(cfi_spi_dma);
DEVFS_DRIVER_DECLARTION(cfi_qspi);
DEVFS_DRIVER_DECLARTION(cfi_qspi_dma);



#endif /* DEVICE_CFI_H_ */
