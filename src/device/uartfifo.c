/* Copyright 2011-2018 Tyler Gilbert; 
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
#include "cortexm/task.h"


static int set_read_action(const devfs_handle_t * handle, mcu_callback_t callback){
	mcu_action_t action;
	action.handler.callback = callback;
	action.handler.context = (void*)handle;
	action.o_events = MCU_EVENT_FLAG_DATA_READY;
	action.prio = 0;
	return mcu_uart_setaction(handle, &action);
}

static int data_received(void * context, const mcu_event_t * data){
	int i;
	const devfs_handle_t * handle;
	const uartfifo_config_t * config;
	uartfifo_state_t * state;
	handle = context;
	config = handle->config;
	state = handle->state;
	int result;
	int size = config->fifo.size;

	result = state->async_read.nbyte;
	do {

		if( result > 0 ){

			//write the new bytes to the buffer
			for(i=0; i < result; i++){
				config->fifo.buffer[ state->fifo.atomic_position.access.head ] = config->read_buffer[i];
				fifo_inc_head(&(state->fifo), size);
			}

			//see if any functions are blocked waiting for data to arrive
			fifo_data_received(&(config->fifo), &(state->fifo));
		}

		state->async_read.nbyte = config->read_buffer_size;
		//what if this returns data right now
		result = mcu_uart_read(handle, &state->async_read);
		if( result < 0 ){
			//fire an error -- set this as an error condition
			return 0;
		}

	} while( result > 0 );

	return 0; //done
}

int uartfifo_open(const devfs_handle_t * handle){
	uartfifo_state_t * state = handle->state;
	fifo_flush(&(state->fifo));
	state->fifo.transfer_handler.read = 0;
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
	int result;
	int count;


	switch(request){
		case I_FIFO_GETINFO:
			fifo_getinfo(ctl, &(config->fifo), &(state->fifo));
			break;
		case I_MCU_SETACTION:
		case I_UART_SETACTION:
			if( action->handler.callback == 0 ){
				//This needs to cancel an ongoing operation
				fifo_cancel_async_read(&(state->fifo));
				return 0;
			}
			return SYSFS_SET_RETURN(ENOTSUP);
		case I_UART_FLUSH:
		case I_FIFO_FLUSH:
			fifo_flush(&(state->fifo));
			return mcu_uart_flush(handle, 0);
		case I_UART_SETATTR:
			if(  (result = mcu_uart_setattr(handle, ctl)) < 0 ){
				return result;
			}
			/* no break */
		case I_FIFO_INIT:
			fifo_flush(&(state->fifo));

			state->async_read.tid = task_get_current();
			state->async_read.flags = 0;
			state->async_read.handler.callback = data_received;
			state->async_read.handler.context = (void*)handle;
			state->async_read.loc = 0;
			state->async_read.buf = config->read_buffer;
			state->async_read.nbyte = config->read_buffer_size;

			count = 0;
			do {
				//flush the USB and get an async call
				result = mcu_uart_read(handle, &state->async_read);
				count++;
			} while( (result > 0) && (count < 128) );
			if ( result < 0 ){ return SYSFS_SET_RETURN(EIO); }
			if( count == 0 ){ return SYSFS_SET_RETURN(EIO); }

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

