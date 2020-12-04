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

#include "device/random.h"
#include "sos/debug.h"
#include <errno.h>
#include <stddef.h>

// 32, 22, 2, 1

static u32 calc_lfsr_next(u32 lfsr) {
  u32 bit;
  bit = ((lfsr >> 0) ^ (lfsr >> 10) ^ (lfsr >> 30) ^ (lfsr >> 31));
  lfsr = (lfsr >> 1) | (bit << 31);
  return lfsr;
}

int random_open(const devfs_handle_t *handle) {
  random_state_t *state = handle->state;
  state->clfsr = 0x55aa55aa; // set the default seed
  return 0;
}

int random_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  random_state_t *state = handle->state;
  random_attr_t *attr = ctl;
  random_info_t *info = ctl;

  if (state == 0) {
    return SYSFS_SET_RETURN(ENOSYS);
  }

  switch (request) {
  case I_RANDOM_GETVERSION:
    return RANDOM_VERSION;

  case I_RANDOM_SETATTR:
    if (attr->o_flags & (RANDOM_FLAG_ENABLE | RANDOM_FLAG_DISABLE)) {
      return 0;
    }

    if (attr->o_flags & RANDOM_FLAG_SET_SEED) {
      // set the seed
      state->clfsr = attr->seed;
      return 0;
    }
    break;

  case I_RANDOM_GETINFO:
    info->o_flags = RANDOM_FLAG_SET_SEED | RANDOM_FLAG_IS_PSEUDO;
    return 0;
  }

  return -EINVAL;
}

int random_read(const devfs_handle_t *handle, devfs_async_t *async) {
  random_state_t *state = handle->state;
  u32 i;
  u8 *dest = async->buf;
  int bytes_read = 0;

  if (state == 0) {
    return SYSFS_SET_RETURN(ENOSYS);
  }

  while (bytes_read < async->nbyte) {
    state->clfsr = calc_lfsr_next(state->clfsr);
    for (i = 0; (i < 4) && (bytes_read < async->nbyte); i++) {
      dest[bytes_read] = state->clfsr >> (i * 8);
      bytes_read++;
    }
  }

  return async->nbyte;
}

int random_write(const devfs_handle_t *handle, devfs_async_t *async) {
  MCU_UNUSED_ARGUMENT(handle);
  MCU_UNUSED_ARGUMENT(async);
  return SYSFS_SET_RETURN(ENOTSUP);
}

int random_close(const devfs_handle_t *handle) {
  MCU_UNUSED_ARGUMENT(handle);
  return 0;
}
