// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_AUTH_H_
#define DEVICE_AUTH_H_

#include "sos/fs/types.h"
#include "sos/dev/auth.h"

int auth_open(const devfs_handle_t * handle);
int auth_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int auth_read(const devfs_handle_t * handle, devfs_async_t * async);
int auth_write(const devfs_handle_t * handle, devfs_async_t * async);
int auth_close(const devfs_handle_t * handle);


#endif /* DEVICE_AUTH_H_ */

