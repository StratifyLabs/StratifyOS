// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


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
