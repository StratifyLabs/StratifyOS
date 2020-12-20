// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "device/null.h"
#include <errno.h>
#include <stddef.h>

int null_open(const devfs_handle_t *handle) { return 0; }

int null_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int null_read(const devfs_handle_t *handle, devfs_async_t *async) {
  return -1; // EOF
}

int null_write(const devfs_handle_t *handle, devfs_async_t *async) {
  // accept the data
  return async->nbyte;
}

int null_close(const devfs_handle_t *handle) { return 0; }
