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
#include "cortexm/task.h"
#include "device/usbfifo.h"
#include "mcu/usb.h"
#include "mcu/debug.h"

static int cancel_read_action(const devfs_handle_t * handle){
	mcu_action_t action;
	const usbfifo_config_t * config = handle->config;

	action.handler.callback = 0;
	action.handler.context = 0;
	action.o_events = MCU_EVENT_FLAG_DATA_READY;
	action.channel = config->endpoint;
	action.prio = 0;
	return mcu_usb_setaction(handle, &action);
}

static int data_received(void * context, const mcu_event_t * data){
	int i;
	const devfs_handle_t * handle;
	const usbfifo_config_t * config;
	usbfifo_state_t * state;
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

		state->async_read.nbyte = config->endpoint_size;
		//what if this returns data right now
		result = mcu_usb_read(handle, &state->async_read);
		if( result < 0 ){
			//fire an error -- set this as an error condition
			return 0;
		}

	} while( result > 0 );

	return 0; //done
}

int usbfifo_open(const devfs_handle_t * handle){
	const usbfifo_config_t * config = handle->config;
	usbfifo_state_t * state = handle->state;

	if( fifo_open_local(&config->fifo, &state->fifo) < 0 ){
		return -1;
	}

	return mcu_usb_open(handle);
}

int usbfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	fifo_info_t * info = ctl;
	mcu_action_t * action = ctl;
	const usbfifo_config_t * config = handle->config;
	usbfifo_state_t * state = handle->state;
	int result;
	switch(request){
		case I_FIFO_GETINFO:
			fifo_getinfo(info, &(config->fifo), &(state->fifo));
			break;
		case I_USB_SETACTION:
		case I_MCU_SETACTION:
			if( action->handler.callback == 0 ){
				fifo_cancel_async_read(&(state->fifo));
			} else {
				return mcu_usb_setaction(handle, ctl);
			}
			return 0;
		case I_FIFO_FLUSH:
			fifo_flush(&(state->fifo));
			devfs_execute_read_handler(&state->fifo.transfer_handler, 0, -1, MCU_EVENT_FLAG_CANCELED);
			break;
		case I_USB_SETATTR:
			fifo_flush(&(state->fifo));
			state->fifo.transfer_handler.read = 0;
			//setup the device to write to the fifo when data arrives

			result = mcu_usb_setattr(handle, ctl);
			if( result < 0 ){ return result; }
			/* no break */
		case I_FIFO_INIT:
			fifo_flush(&(state->fifo));
			state->async_read.tid = task_get_current();
			state->async_read.flags = 0;
			state->async_read.handler.callback = data_received;
			state->async_read.handler.context = (void*)handle;
			state->async_read.loc = config->endpoint;
			state->async_read.buf = config->read_buffer;
			state->async_read.nbyte = config->endpoint_size;

			result = mcu_usb_read(handle, &state->async_read);
			if ( result < 0 ){ return SYSFS_SET_RETURN(EIO); }

			break;
		case I_FIFO_EXIT:
			//clear the callback for the device
			result = cancel_read_action(handle);
			if( result < 0 ){ return SYSFS_SET_RETURN(EIO); }
			return mcu_usb_close(handle);
		default:
			return mcu_usb_ioctl(handle, request, ctl);
	}
	return 0;
}


int usbfifo_read(const devfs_handle_t * handle, devfs_async_t * async){
	const usbfifo_config_t * config = handle->config;
	usbfifo_state_t * state = handle->state;
	return fifo_read_local(&(config->fifo), &(state->fifo), async, 1);
}

int usbfifo_write(const devfs_handle_t * handle, devfs_async_t * async){
	const usbfifo_config_t * config = handle->config;
	async->loc = 0x80 | config->endpoint;
	//Writing to the USB FIFO is not buffered, it just writes the USB HW directly
	return mcu_usb_write(handle, async);
}

int usbfifo_close(const devfs_handle_t * handle){
	//use I_FIFO_EXIT to close the USB
	return 0;
}

