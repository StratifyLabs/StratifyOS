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


#if !defined __link

#include "mcu/debug.h"
#include "sos/dev/uart.h"
#include "mcu/uart.h"
#include "mcu/core.h"
#include "mcu/mcu.h"


#if MCU_DEBUG

char mcu_debug_buffer[MCU_DEBUG_BUFFER_SIZE];

int mcu_debug_init(){
	int port;
	uart_attr_t attr;

	//Open the debugging UART
	port = MCU_DEBUG_PORT;
	if( mcu_uart_open((devfs_handle_t*)&port) < 0 ){
		return -1;
	}

	memset(attr.pin_assignment, 0xff, UART_PIN_ASSIGNMENT_COUNT*sizeof(mcu_pin_t));
	attr.pin_assignment[0] = mcu_board_config.debug_uart_pin_assignment[0];
	attr.pin_assignment[1] = mcu_board_config.debug_uart_pin_assignment[1];
	attr.freq = 115200;
	attr.o_flags = UART_FLAG_IS_PARITY_NONE | UART_FLAG_IS_STOP1;
	attr.width = 8;

	return mcu_uart_setattr(port, &attr);
}

static int write_done(void * context, mcu_event_t data){
	int * busy = (int*)context;
	*busy = 0;
	return 0;
}


void mcu_priv_write_debug_uart(void * args){
	int nbyte;
	int err;
	volatile int busy;
	devfs_async_t wop;
	int port = MCU_DEBUG_PORT;
	wop.loc = 0;
	wop.buf = mcu_debug_buffer;
	wop.nbyte = 0;
	wop.handler.callback = write_done;
	wop.handler.context = (void*)&busy;
	nbyte = strlen(mcu_debug_buffer);
	wop.nbyte = nbyte;
	busy = 1;

	if ( (err = mcu_uart_write((devfs_handle_t*)&port, &wop)) == 0 ){
		while(busy == 1){
			;
		}
	} else {
		if( err < 0 ){
			while(1);
		}
	}
}

#endif

#endif






