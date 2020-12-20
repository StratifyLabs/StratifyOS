// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_NULL_H_
#define MCU_NULL_H_

#include "sos/fs/devfs.h"

int null_open(const devfs_handle_t * handle);
int null_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int null_read(const devfs_handle_t * handle, devfs_async_t * rop);
int null_write(const devfs_handle_t * handle, devfs_async_t * wop);
int null_close(const devfs_handle_t * handle);

#endif /* MCU_NULL_H_ */
