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

#include "mcu/debug.h"
#include "iface/dev/uart.h"
#include "mcu/uart.h"

#if defined MCU_DEBUG

char mcu_debug_buffer[MCU_DEBUG_BUFFER_SIZE];

int mcu_debug_init(void){
	int port;
	int err;
	uart_attr_t uart_cfg;

	//Open the debugging UART
	port = 0;
	err = mcu_uart_open((device_cfg_t*)&port);
	if ( err < 0 ){
		return err;
	}


	uart_cfg.pin_assign = 0;
	uart_cfg.baudrate = 115200;
	uart_cfg.parity = UART_PARITY_NONE;
	uart_cfg.stop = UART_ATTR_STOP_BITS_1;
	uart_cfg.start = 0;
	uart_cfg.width = 8;

	err = mcu_uart_ioctl((device_cfg_t*)&port, I_UART_SETATTR, &uart_cfg);

	if ( err < 0 ){
		return err;
	}

	return err;
}


void mcu_priv_write_debug_uart(void * args){
	device_transfer_t wop;
	int port = 0;
	wop.loc = 0;
	wop.buf = NULL;
	wop.nbyte = 0;
	wop.callback = NULL;
	while( mcu_uart_write((device_cfg_t*)&port, &wop) == 0 );
	wop.buf = mcu_debug_buffer;
	wop.nbyte = strlen(mcu_debug_buffer);
	if ( mcu_uart_write((device_cfg_t*)&port, &wop) == 0 ){
		wop.nbyte = 0;
		wop.buf = NULL;
		while( mcu_uart_write((device_cfg_t*)&port, &wop) == 0 );
	}
}

#endif







