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

#ifndef MCU_RANDOM_H_
#define MCU_RANDOM_H_

#include "sos/dev/random.h"
#include "sos/fs/devfs.h"

typedef struct {
    u32 clfsr;
} random_state_t;

int random_open(const devfs_handle_t * handle);
int random_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int random_read(const devfs_handle_t * handle, devfs_async_t * rop);
int random_write(const devfs_handle_t * handle, devfs_async_t * wop);
int random_close(const devfs_handle_t * handle);





#endif /* MCU_RANDOM_H_ */
