/* Copyright 2011-2016 Tyler Gilbert; 
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

#include "mcu/mcu.h"
#include "mcu/uart.h"

#include "sos/dev/tty.h"

static const int tty_speeds[B921600 + 1] = {
		0,
		50,
		75,
		110,
		134,
		150,
		200,
		300,
		600,
		1200,
		2400,
		4800,
		9600,
		19200,
		38400,
		57600,
		115200,
		230400,
		4600800,
		921600
};


int tty_uart_open(const devfs_handle_t * cfg){
	//make sure the port is open and configured the way we want
	return 0;
}

int tty_uart_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	uart_attr_t attr;
	int tmp;
	struct termios * p = (struct termios*)ctl;

	attr.o_flags = 0;

	switch(request){
	case I_TTY_SETATTR:

		if ( p->c_ispeed != p->c_ospeed ){
			return -1;
		}

		tmp = p->c_ospeed;
		if ( tmp < (B921600 + 1) ){
			attr.freq = tty_speeds[tmp];
		} else {
			return -1;
		}

		tmp = p->c_cflag & CSIZE;
		if ( 1 ){
			switch(tmp){
			case CS5:
				attr.width = 5;
				break;
			case CS6:
				attr.width = 6;
				break;
			case CS7:
				attr.width = 7;
				break;
			case CS8:
				attr.width = 8;
				break;

			}
		}

		if ( p->c_cflag & CSTOPB ){
			attr.o_flags |= UART_FLAG_IS_STOP2;
		} else {
			attr.o_flags |= UART_FLAG_IS_STOP1;
		}

		if ( p->c_cflag & PARENB ){
			if ( p->c_cflag & PARODD){
				attr.o_flags |= UART_FLAG_IS_PARITY_ODD;
			} else {
				attr.o_flags |= UART_FLAG_IS_PARITY_EVEN;
			}
		} else {
			attr.o_flags |= UART_FLAG_IS_PARITY_NONE;
		}

		return mcu_uart_setattr(handle, &attr);

		break;
	}

	return 0;
}

int tty_uart_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return mcu_uart_write(cfg, wop);
}

int tty_uart_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_uart_read(cfg, rop);
}

int tty_uart_close(const devfs_handle_t * cfg){
	return 0;
}
