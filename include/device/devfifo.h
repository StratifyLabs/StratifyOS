// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEV_DEVFIFO_H_
#define DEV_DEVFIFO_H_

#include "sos/dev/devfifo.h"
#include "sos/fs/devfs.h"

/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
	int head;
	int tail;
	bool overflow;
	devfs_async_t * rop;
	int len;
} devfifo_state_t;

int devfifo_open(const devfs_handle_t * handle);
int devfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int devfifo_read(const devfs_handle_t * handle, devfs_async_t * rop);
int devfifo_write(const devfs_handle_t * handle, devfs_async_t * wop);
int devfifo_close(const devfs_handle_t * handle);


#endif /* DEV_DEVFIFO_H_ */

