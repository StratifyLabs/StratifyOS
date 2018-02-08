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

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "device/uartfifo.h"
#include "mcu/debug.h"


static int set_read_action(const devfs_handle_t * handle, mcu_callback_t callback){
	mcu_action_t action;
	action.handler.callback = callback;
	action.handler.context = (void*)handle;
	action.o_events = MCU_EVENT_FLAG_DATA_READY;
	action.prio = 0;
	return mcu_uart_setaction(handle, &action);
}

static int data_received(void * context, const mcu_event_t * data){
	char c;
	const devfs_handle_t * handle;
	const uartfifo_config_t * cfgp;
	uartfifo_state_t * state;
	handle = context;
	cfgp = handle->config;
	state = handle->state;

	while( mcu_uart_get(handle, &c) == 0 ){
		cfgp->fifo.buffer[ state->fifo.head ] = c;
		fifo_inc_head(&(state->fifo), cfgp->fifo.size);
	}

	fifo_data_received(&(cfgp->fifo), &(state->fifo));
	return 1; //leave the callback in place
}

int uartfifo_open(const devfs_handle_t * handle){
	uartfifo_state_t * state = handle->state;
	fifo_flush(&(state->fifo));
	state->fifo.read_async = NULL;
	//setup the device to write to the fifo when data arrives
	if( mcu_uart_open(handle) < 0 ){
		return -1;
	}

	return 0;
}

int uartfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	mcu_action_t * action = ctl;
	const uartfifo_config_t * config = handle->config;
	uartfifo_state_t * state = handle->state;
	int ret;

	switch(request){
	case I_FIFO_GETINFO:
		fifo_getinfo(ctl, &(config->fifo), &(state->fifo));
		break;
	case I_MCU_SETACTION:
	case I_UART_SETACTION:
		if( action->handler.callback == 0 ){
			//This needs to cancel an ongoing operation
			fifo_cancel_rop(&(state->fifo));
			return 0;
		}
		errno = ENOTSUP;
		return -1;
	case I_UART_FLUSH:
	case I_FIFO_FLUSH:
		fifo_flush(&(state->fifo));
		return mcu_uart_flush(handle, 0);
	case I_UART_SETATTR:
		if(  (ret = mcu_uart_setattr(handle, ctl)) < 0 ){
			return ret;
		}
		/* no break */
	case I_FIFO_INIT:
		fifo_flush(&(state->fifo));
		if ( set_read_action(handle, data_received) < 0 ){
			return -1;
		}
		break;
	default:
		return mcu_uart_ioctl(handle, request, ctl);
	}
	return 0;
}


int uartfifo_read(const devfs_handle_t * handle, devfs_async_t * rop){
	const uartfifo_config_t * cfgp = handle->config;
	uartfifo_state_t * state = handle->state;
    return fifo_read_local(&(cfgp->fifo), &(state->fifo), rop, 1);
}

int uartfifo_write(const devfs_handle_t * handle, devfs_async_t * wop){

	//FIFO is not used for writing; hardware is written directly
	return mcu_uart_write(handle, wop);
}

int uartfifo_close(const devfs_handle_t * handle){

	//clear the callback for the device
	if( set_read_action(handle, 0) < 0 ){
		return -1;
	}

	return mcu_uart_close(handle);
}

