/* Copyright 2011-2019 Tyler Gilbert;
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

#ifndef DEVICE_DRIVE_RAM_H_
#define DEVICE_DRIVE_RAM_H_

#include "sos/fs/devfs.h"
#include "sos/dev/drive.h"

typedef struct {
	void * memory;
	u32 size;
} drive_ram_config_t;


DEVFS_DRIVER_DECLARTION(drive_ram);


#endif /* DEVICE_DRIVE_RAM_H_ */
