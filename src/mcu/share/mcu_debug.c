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

#include <stdarg.h>

#include "cortexm/cortexm.h"
#include "sos/sos.h"
#include "mcu/debug.h"
#include "sos/dev/uart.h"
#include "mcu/uart.h"
#include "mcu/core.h"
#include "mcu/mcu.h"


#if MCU_DEBUG

typedef struct {
	char buffer[256];
	int len;
} mcu_debug_buffer_t;

static void mcu_debug_root_write_uart_svcall(void * args);

int mcu_debug_init(){
	devfs_handle_t handle;

	//Open the debugging UART
	handle.port = mcu_board_config.debug_uart_port;
	handle.config = 0;
	handle.state = 0;
	if( mcu_uart_open(&handle) < 0 ){
		return -1;
	}

	return mcu_uart_setattr(&handle, (void*)&mcu_board_config.debug_uart_attr);
}

void mcu_debug_root_write_uart(const char * buffer, int nbyte){
	int i;
	devfs_handle_t handle;
	handle.port = mcu_board_config.debug_uart_port;
	handle.config = 0;
	handle.state = 0;

	for(i=0; i < nbyte; i++){
		mcu_uart_put(&handle, (void*)(u32)(buffer[i]));
	}
}

void mcu_debug_root_write_uart_svcall(void * args){
	mcu_debug_buffer_t * p = args;
	mcu_debug_root_write_uart(p->buffer, p->len);
}

int mcu_debug_user_printf(const char * format, ...){
	mcu_debug_buffer_t svcall_args;
	va_list args;
	va_start (args, format);
	svcall_args.buffer[255] = 0;
	svcall_args.len = vsnprintf(svcall_args.buffer, 255, format, args);
	cortexm_svcall(mcu_debug_root_write_uart_svcall, &svcall_args);
	return svcall_args.len;
}

int mcu_debug_root_printf(const char * format, ...){
	int ret = 0;
	char buffer[256];
	buffer[255] = 0;
	va_list args;
	va_start (args, format);
	ret = vsnprintf(buffer, 255, format, args);
	mcu_debug_root_write_uart(buffer, ret);
	return ret;
}

#endif

#endif






