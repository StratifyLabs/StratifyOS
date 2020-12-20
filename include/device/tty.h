// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEV_TTY_H_
#define DEV_TTY_H_

#include "sos/fs/devfs.h"

int tty_usbbulk_open(const devfs_handle_t * handle);
int tty_usbbulk_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int tty_usbbulk_write(const devfs_handle_t * handle, devfs_async_t * wop);
int tty_usbbulk_read(const devfs_handle_t * handle, devfs_async_t * rop);
int tty_usbbulk_close(const devfs_handle_t * handle);

int tty_uart_open(const devfs_handle_t * handle);
int tty_uart_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int tty_uart_write(const devfs_handle_t * handle, devfs_async_t * wop);
int tty_uart_read(const devfs_handle_t * handle, devfs_async_t * rop);
int tty_uart_close(const devfs_handle_t * handle);


#endif /* DEV_TTY_H_ */
