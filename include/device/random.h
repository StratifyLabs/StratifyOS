// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_RANDOM_H_
#define MCU_RANDOM_H_

#include "sos/dev/random.h"
#include "sos/fs/devfs.h"

typedef struct {
    u32 clfsr;
} random_state_t;

int random_open(const devfs_handle_t * handle);
int random_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int random_read(const devfs_handle_t * handle, devfs_async_t * rop);
int random_write(const devfs_handle_t * handle, devfs_async_t * wop);
int random_close(const devfs_handle_t * handle);





#endif /* MCU_RANDOM_H_ */
