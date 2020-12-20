// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef TERMIOS_H_
#define TERMIOS_H_


#include <termios.h>


int termios_usbbulk_open(const devfs_handle_t * handle);
int termios_usbbulk_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int termios_usbbulk_write(const devfs_handle_t * handle, devfs_async_t * wop);
int termios_usbbulk_read(const devfs_handle_t * handle, devfs_async_t * rop);
int termios_usbbulk_close(const devfs_handle_t * handle);

int termios_uart_open(const devfs_handle_t * handle);
int termios_uart_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int termios_uart_write(const devfs_handle_t * handle, devfs_async_t * wop);
int termios_uart_read(const devfs_handle_t * handle, devfs_async_t * rop);
int termios_uart_close(const devfs_handle_t * handle);


#endif /* TERMIOS_H_ */
