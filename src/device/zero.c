// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "device/zero.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>

int zero_open(const devfs_handle_t *handle) { return 0; }

int zero_ioctl(const devfs_handle_t *handle, int request, void *ctl) { return 0; }

int zero_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  memset(rop->buf, 0, rop->nbyte);
  return rop->nbyte;
}

int zero_write(const devfs_handle_t *handle, devfs_async_t *wop) { return wop->nbyte; }

int zero_close(const devfs_handle_t *handle) { return 0; }
