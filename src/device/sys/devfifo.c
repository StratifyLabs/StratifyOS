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
#include "sos/dev/devfifo.h"
#include "mcu/devfifo.h"
#include "mcu/debug.h"


static int set_read_action(const devfs_handle_t * cfg, mcu_callback_t callback){
	mcu_action_t action;
	const devfifo_config_t * cfgp = cfg->config;
	const devfs_device_t * device = cfgp->dev;

	action.handler.callback = callback;
	action.handler.context = (void*)cfg;
	action.o_events = cfgp->event;
	action.prio = 0;
	if ( device->driver.ioctl(&(device->handle), cfgp->req_setaction, &action) < 0 ){
		return -1;
	}

	return 0;
}

static void inc_head(devfifo_state_t * state, int size){
	state->head++;
	if ( state->head == size ){
		state->head = 0;
	}

	if ( state->head == state->tail ){
		state->tail++;
		if ( state->tail == size ){
			state->tail = 0;
		}
		state->overflow = true;
	}
}

static int read_buffer(const devfifo_config_t * cfgp, devfifo_state_t * state, devfs_async_t * rop){
	int i;
	for(i=0; i < state->len; i++){
		if ( state->head == state->tail ){ //check for data in the fifo buffer
			break;
		} else {
			((char*)(rop->buf))[i] = cfgp->buffer[state->tail];
			state->tail++;
			if ( state->tail == cfgp->size ){
				state->tail = 0;
			}
		}
	}
	return i; //number of bytes read
}

static int data_received(void * context, mcu_event_t * data){
	char c;
	int bytes_read;
	const devfs_handle_t * cfg;
	cfg = context;
	const devfifo_config_t * cfgp = cfg->config;
	devfifo_state_t * state = cfg->state;
	const devfs_device_t * device = cfgp->dev;

	while ( device->driver.ioctl(&(device->handle), cfgp->req_getbyte, &c) == 0 ){
		cfgp->buffer[ state->head ] = c;
		inc_head(state, cfgp->size);
	}

	if( state->rop != NULL ){
		if( (bytes_read = read_buffer(cfgp, state, state->rop)) > 0 ){
			state->rop->nbyte = bytes_read;
			if ( state->rop->handler.callback(state->rop->handler.context, NULL) == 0 ){
				state->rop = NULL;
			}
		}
	}


	return 1; //leave the callback in place
}

int devfifo_open(const devfs_handle_t * cfg){
	const devfifo_config_t * cfgp = cfg->config;
	devfifo_state_t * state = cfg->state;
	const devfs_device_t * device = cfgp->dev;
	state->head = 0;
	state->tail = 0;
	state->rop = NULL;
	state->overflow = false;
	//setup the device to write to the fifo when data arrives
	if ( device->driver.open(&(device->handle)) < 0 ){
		return -1;
	}

	if ( set_read_action(cfg, data_received) < 0 ){
		return -1;
	}

	return 0;
}

int devfifo_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	devfifo_attr_t * attr = ctl;
	const devfifo_config_t * cfgp = cfg->config;
	devfifo_state_t * state = cfg->state;
	const devfs_device_t * device = cfgp->dev;

	if ( request == I_DEVFIFO_GETINFO ){
		attr->size = cfgp->size;
		if( state->head > state->tail ){
			attr->used = state->head - state->tail;
		} else {
			attr->used = cfgp->size - state->tail + state->head;
		}
		attr->overflow = state->overflow;
		state->overflow = false; //clear the overflow flag now that it has been read
		return 0;
	} else {
		return device->driver.ioctl(&(device->handle), request, ctl);
	}
}


int devfifo_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	const devfifo_config_t * cfgp = cfg->config;
	devfifo_state_t * state = cfg->state;

	int bytes_read;

	if ( state->rop != NULL ){
		rop->nbyte = 0;
		return 0; //caller will block until data is read
	}

	state->len = rop->nbyte;
	bytes_read = read_buffer(cfgp, state, rop); //see if there are bytes in the buffer
	if ( bytes_read == 0 ){
		if( (rop->flags & O_NONBLOCK) ){
			errno = EAGAIN;
			bytes_read = -1;
		} else {
			state->rop = rop;
			state->len = rop->nbyte;
			rop->nbyte = 0;
		}
	}

	return bytes_read;
}

int devfifo_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	const devfifo_config_t * cfgp = cfg->config;
	const devfs_device_t * device = cfgp->dev;

	return device->driver.write(&(device->handle), wop);
}

int devfifo_close(const devfs_handle_t * cfg){
	const devfifo_config_t * cfgp = cfg->config;
	const devfs_device_t * device = cfgp->dev;

	//clear the callback for the device
	if( set_read_action(cfg, NULL) < 0 ){
		return -1;
	}

	return device->driver.close(&(device->handle));
}

