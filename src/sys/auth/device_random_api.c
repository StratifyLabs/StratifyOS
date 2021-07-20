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

#include <sdk/api.h>

#include "config.h"
#include "sos/api/crypt_api.h"
#include "sos/dev/random.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int fd;
} device_random_context_t;

static int set_device_attributes(device_random_context_t *context, u32 o_flags, u32 seed);

static int device_random_init(void **context) {
  int fd = open("/dev/random", O_RDWR);
  if (fd < 0) {
    return -1;
  }

  device_random_context_t *c = malloc(sizeof(device_random_context_t));
  if (c == NULL) {
    close(fd);
    return -1;
  }

  c->fd = fd;

  *context = c;

  return set_device_attributes(c, RANDOM_FLAG_ENABLE, 0);
}

static void device_random_deinit(void **context) {
  if (*context != 0) {
    void *c_ptr = *context;
    device_random_context_t *c = c_ptr;
    close(c->fd);
    // zero out the context
    c->fd = -1;
    *context = 0;
    free(c_ptr);
  }
}

static int device_random_seed(void *context, const unsigned char *data, u32 data_len) {
  u32 seed = 0;
  data_len = data_len > sizeof(u32) ? sizeof(u32) : data_len;
  memcpy(&seed, data, data_len);
  return set_device_attributes(context, RANDOM_FLAG_SET_SEED, seed);
}

static int device_random_random(void *context, unsigned char *output, u32 output_length) {
  device_random_context_t *c = context;
  return read(c->fd, output, output_length);
}

static int set_device_attributes(device_random_context_t *context, u32 o_flags, u32 seed) {
  random_attr_t attributes;

  attributes.o_flags = o_flags;
  attributes.seed = seed;
  return ioctl(context->fd, I_RANDOM_SETATTR, &attributes);
}

const crypt_random_api_t device_random_api = {
  .sos_api = {.name = "crypt_random_device", .version = 0x0001, .git_hash = SOS_GIT_HASH},
  .init = device_random_init,
  .deinit = device_random_deinit,
  .seed = device_random_seed,
  .random = device_random_random};
