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
#include "device/usbfifo.h"
#include "mcu/usb.h"
#include "mcu/debug.h"


static int set_read_action(const devfs_handle_t * handle, mcu_callback_t callback){
	mcu_action_t action;
	const usbfifo_config_t * cfgp = handle->config;
	action.handler.callback = callback;
	action.handler.context = (void*)handle;
	action.o_events = MCU_EVENT_FLAG_DATA_READY;
	action.channel = cfgp->endpoint;
	action.prio = 0;
	if( mcu_usb_setaction(handle, &action) < 0 ){
		return -1;
	}

	return 0;
}

static int data_received(void * context, mcu_event_t * data){
	int i;
	int bytes_read;
	const devfs_handle_t * handle;
	const usbfifo_config_t * config;
	usbfifo_state_t * state;
	handle = context;
	config = handle->config;
	state = handle->state;
	int size = config->fifo.size;
	char buffer[config->endpoint_size];

	//check to see if USB was disconnected
	if( mcu_usb_isconnected(handle, NULL) ){


		//read the endpoint directly
		bytes_read = mcu_usb_rd_ep(handle, config->endpoint, buffer);
		if( bytes_read > config->endpoint_size){
			bytes_read = config->endpoint_size;
		}

		//write the new bytes to the buffer
		for(i=0; i < bytes_read; i++){
			config->fifo.buffer[ state->fifo.head ] = buffer[i];
			fifo_inc_head(&(state->fifo), size);
		}


		//see if any functions are blocked waiting for data to arrive
		fifo_data_received(&(config->fifo), &(state->fifo));

	}

	return 1; //leave the callback in place
}

int usbfifo_open(const devfs_handle_t * cfg){
	return mcu_usb_open(cfg);
}

int usbfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	fifo_info_t * info = ctl;
	mcu_action_t * action = ctl;
	const usbfifo_config_t * cfgp = handle->config;
	usbfifo_state_t * state = handle->state;
	mcu_event_t event;
	switch(request){
	case I_FIFO_GETINFO:
		fifo_getinfo(info, &(cfgp->fifo), &(state->fifo));
		break;
	case I_USB_SETACTION:
	case I_MCU_SETACTION:
		if( action->handler.callback == 0 ){
			fifo_cancel_rop(&(state->fifo));
		} else {
			return mcu_usb_setaction(handle, ctl);
		}
		return 0;
	case I_FIFO_FLUSH:
		fifo_flush(&(state->fifo));
		if ( state->fifo.rop != NULL ){
			state->fifo.rop->nbyte = -1;
			event.o_events = MCU_EVENT_FLAG_CANCELED;
			event.data = 0;
			if ( state->fifo.rop->handler.callback(state->fifo.rop->handler.context, &event) == 0 ){
				state->fifo.rop = NULL;
			}
		}
		state->fifo.rop = NULL;
		break;
	case I_USB_SETATTR:
		fifo_flush(&(state->fifo));
		state->fifo.rop = NULL;
		//setup the device to write to the fifo when data arrives

		if(  mcu_usb_setattr(handle, ctl) < 0 ){
			return -1;
		}
		/* no break */
	case I_FIFO_INIT:
		fifo_flush(&(state->fifo));
		if ( set_read_action(handle, data_received) < 0 ){
			return -1;
		}
		break;
	case I_FIFO_EXIT:
		//clear the callback for the device
		if( set_read_action(handle, NULL) < 0 ){
			return -1;
		}
		return mcu_usb_close(handle);
	default:
		return mcu_usb_ioctl(handle, request, ctl);
	}
	return 0;
}


int usbfifo_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	const usbfifo_config_t * cfgp = cfg->config;
	usbfifo_state_t * state = cfg->state;
	return fifo_read_local(&(cfgp->fifo), &(state->fifo), rop);
}

int usbfifo_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	const usbfifo_config_t * cfgp = cfg->config;
	wop->loc = 0x80 | cfgp->endpoint;

	//Writing to the USB FIFO is not buffered, it just writes the USB HW directly
	return mcu_usb_write(cfg, wop);
}

int usbfifo_close(const devfs_handle_t * cfg){
	//use I_FIFO_EXIT to close the USB
	return 0;
}

