// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_URANDOM_H_
#define MCU_URANDOM_H_

#include "sos/fs/devfs.h"

int urandom_open(const devfs_handle_t * handle);
int urandom_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int urandom_read(const devfs_handle_t * handle, devfs_async_t * rop);
int urandom_write(const devfs_handle_t * handle, devfs_async_t * wop);
int urandom_close(const devfs_handle_t * handle);




#endif /* MCU_URANDOM_H_ */
