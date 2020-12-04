/* Copyright 2011-2018 Tyler Gilbert;
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

#include "device/urandom.h"
#include <errno.h>
#include <stddef.h>

int urandom_open(const devfs_handle_t *handle) { return 0; }

int urandom_ioctl(const devfs_handle_t *handle, int request, void *ctl) { return 0; }

int urandom_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int urandom_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int urandom_close(const devfs_handle_t *handle) { return 0; }
