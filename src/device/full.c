// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "device/full.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>

int full_open(const devfs_handle_t *handle) { return 0; }

int full_ioctl(const devfs_handle_t *handle, int request, void *ctl) { return 0; }

int full_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  memset(rop->buf, 0, rop->nbyte);
  return rop->nbyte;
}

int full_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  return SYSFS_SET_RETURN(ENOSPC);
}

int full_close(const devfs_handle_t *handle) { return 0; }
