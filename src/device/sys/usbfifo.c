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
#include "sos/dev/usbfifo.h"
#include "mcu/usbfifo.h"
#include "mcu/usb.h"
#include "mcu/debug.h"


static int set_read_action(const devfs_handle_t * cfg, mcu_callback_t callback){
	mcu_action_t action;
	const usbfifo_cfg_t * cfgp = cfg->config;
	action.handler.callback = callback;
	action.handler.context = (void*)cfg;
	action.o_events = USB_EVENT_DATA_READY;
	action.channel = cfgp->endpoint;
	action.prio = 0;
	if( mcu_usb_setaction(cfgp->port, &action) < 0 ){
		return -1;
	}

	return 0;
}

static int data_received(void * context, mcu_event_t data){
	int i;
	int bytes_read;
	const devfs_handle_t * cfg;
	const usbfifo_cfg_t * cfgp;
	usbfifo_state_t * state;
	cfg = context;
	cfgp = cfg->config;
	state = cfg->state;
	int size = cfgp->fifo.size;
	char buffer[cfgp->endpoint_size];

	//check to see if USB was disconnected
	if( mcu_usb_isconnected(cfgp->port, NULL) ){

		//read the endpoint directly
		bytes_read = mcu_usb_rd_ep(cfgp->port, cfgp->endpoint, buffer);

		for(i=0; i < bytes_read; i++){
			cfgp->fifo.buffer[ state->fifo.head ] = buffer[i];
			fifo_inc_head(&(state->fifo), size);
		}

		fifo_data_received(&(cfgp->fifo), &(state->fifo));
	}

	return 1; //leave the callback in place
}

int usbfifo_open(const devfs_handle_t * cfg){
	const usbfifo_cfg_t * cfgp = cfg->config;
	return mcu_usb_open((const devfs_handle_t*)&(cfgp->port));
}

int usbfifo_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	fifo_info_t * info = ctl;
	mcu_action_t * action = ctl;
	const usbfifo_cfg_t * cfgp = cfg->config;
	usbfifo_state_t * state = cfg->state;
	switch(request){
	case I_FIFO_GETINFO:
		fifo_getinfo(info, &(cfgp->fifo), &(state->fifo));
		break;
	case I_USB_SETACTION:
	case I_MCU_SETACTION:
		if( action->handler.callback == 0 ){
			fifo_cancel_rop(&(state->fifo));
		} else {
			return mcu_usb_setaction(cfgp->port, ctl);
		}
		return 0;
	case I_FIFO_FLUSH:
		fifo_flush(&(state->fifo));
		if ( state->fifo.rop != NULL ){
			state->fifo.rop->nbyte = -1;
			if ( state->fifo.rop->handler.callback(state->fifo.rop->handler.context, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED)) == 0 ){
				state->fifo.rop = NULL;
			}
		}
		state->fifo.rop = NULL;
		break;
	case I_USB_SETATTR:
		fifo_flush(&(state->fifo));
		state->fifo.rop = NULL;
		//setup the device to write to the fifo when data arrives

		if(  mcu_usb_setattr(cfgp->port, ctl) < 0 ){
			return -1;
		}
		/* no break */
	case I_FIFO_INIT:
		fifo_flush(&(state->fifo));
		if ( set_read_action(cfg, data_received) < 0 ){
			return -1;
		}
		break;
	case I_FIFO_EXIT:
		//clear the callback for the device
		if( set_read_action(cfg, NULL) < 0 ){
			return -1;
		}
		return mcu_usb_close((const devfs_handle_t*)&(cfgp->port));
	default:
		return mcu_usb_ioctl((const devfs_handle_t*)&(cfgp->port), request, ctl);
	}
	return 0;
}


int usbfifo_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	const usbfifo_cfg_t * cfgp = cfg->config;
	usbfifo_state_t * state = cfg->state;
	return fifo_read_local(&(cfgp->fifo), &(state->fifo), rop);
}

int usbfifo_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	const usbfifo_cfg_t * cfgp = cfg->config;
	wop->loc = 0x80 | cfgp->endpoint;

	//Writing to the USB FIFO is not buffered, it just writes the USB HW directly
	return mcu_usb_write((const devfs_handle_t*)&(cfgp->port), wop);
}

int usbfifo_close(const devfs_handle_t * cfg){
	//use I_FIFO_EXIT to close the USB
	return 0;
}

