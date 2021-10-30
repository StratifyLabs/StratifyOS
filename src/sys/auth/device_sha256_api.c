// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <sdk/api.h>

#include "config.h"
#include "device/auth.h"
#include "sos/api/crypt_api.h"
#include "sos/dev/hash.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int fd;
  u8 digest[32];
} device_sha256_context_t;

int device_sha256_init(void **context) {
  int fd = open("/dev/hash0", O_RDWR);
  if (fd < 0) {
    return -1;
  }

  void *c = malloc(sizeof(device_sha256_context_t));
  if (c == 0) {
    close(fd);
    return -1;
  }

  ((device_sha256_context_t *)c)->fd = fd;

  *context = c;
  return 0;
}

void device_sha256_deinit(void **context) {
  if (*context != 0) {
    void *c_ptr = *context;
    device_sha256_context_t *c = c_ptr;
    close(c->fd);
    c->fd = -1;
    *context = 0;
    free(c_ptr);
  }
}

int device_sha256_start(void *context) {
  // device_sha256_context_t * c = context;
  // use dev/hash to start a new hash ioctl to re-init

  return 0;
}

int device_sha256_update(void *context, const unsigned char *input, u32 size) {
  device_sha256_context_t *c = context;
  int result = write(c->fd, input, size);
  if (result < 0) {
    return result;
  }

  return 0;
}

int device_sha256_finish(void *context, unsigned char *output, u32 size) {
  device_sha256_context_t *c = context;

  if (size < 32) {
    errno = EINVAL;
    return -1;
  }

  int result = read(c->fd, output, 32);
  if (result < 0) {
    return result;
  }

  return 0;
}

const crypt_hash_api_t device_sha256_hash_api = {
  .sos_api = {.name = "device_sha256_hash", .version = 0x0001, .git_hash = SOS_GIT_HASH},
  .init = device_sha256_init,
  .deinit = device_sha256_deinit,
  .start = device_sha256_start,
  .update = device_sha256_update,
  .finish = device_sha256_finish};
