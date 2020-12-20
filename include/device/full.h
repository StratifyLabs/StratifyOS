// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_FULL_H_
#define MCU_FULL_H_

#include "sos/fs/devfs.h"


int full_open(const devfs_handle_t * handle);
int full_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int full_read(const devfs_handle_t * handle, devfs_async_t * rop);
int full_write(const devfs_handle_t * handle, devfs_async_t * wop);
int full_close(const devfs_handle_t * handle);


#endif /* MCU_FULL_H_ */
