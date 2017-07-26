/* Copyright 2011-2017 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef TERMIOS_H_
#define TERMIOS_H_


#include <termios.h>


int termios_usbbulk_open(const devfs_handle_t * cfg);
int termios_usbbulk_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int termios_usbbulk_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int termios_usbbulk_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int termios_usbbulk_close(const devfs_handle_t * cfg);

int termios_uart_open(const devfs_handle_t * cfg);
int termios_uart_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int termios_uart_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int termios_uart_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int termios_uart_close(const devfs_handle_t * cfg);


#endif /* TERMIOS_H_ */
