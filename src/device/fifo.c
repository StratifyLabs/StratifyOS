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
#include "mcu/debug.h"
#include "device/fifo.h"

static void execute_write_callback(fifo_state_t * state, int nbyte, u32 o_events);
static void execute_read_callback(fifo_state_t * state, int nbyte, u32 o_events);


void execute_read_callback(fifo_state_t * state, int nbyte, u32 o_events){
    if( state->read_async != NULL ){
        state->read_async->nbyte = nbyte;
        mcu_execute_event_handler(&(state->read_async->handler), o_events, 0);
        if( state->read_async->handler.callback == 0 ){
            state->read_async = 0;
		}
	}
}

void execute_write_callback(fifo_state_t * state, int nbyte, u32 o_events){
    if( state->write_async != NULL ){
        state->write_async->nbyte = nbyte;
        mcu_execute_event_handler(&(state->write_async->handler), o_events, 0);
        if( state->write_async->handler.callback == 0 ){
            state->write_async = 0;
		}
	}
}


void fifo_inc_tail(fifo_state_t * state, int size){
	state->tail++;
	if ( state->tail == size ){
		state->tail = 0;
	}
}

void fifo_inc_head(fifo_state_t * state, int size){
	state->head++;
	if ( state->head == size ){
		state->head = 0;
	}

	if ( state->head == state->tail ){
		state->tail++;
		if ( state->tail == size ){
			state->tail = 0;
		}
		fifo_set_overflow(state, 1);
	}
}

int fifo_is_write_ok(fifo_state_t * state, u16 size, int writeblock){
	int ret = 1;
	if( ((state->tail == 0) && (state->head == size -1)) ||
			((state->head) + 1 == state->tail)
	){
		if( writeblock ){
			//cannot write anymore data at this time
			ret = 0;
		} else {
			//OK to write but it will cause an overflow
			fifo_inc_tail(state, size);
			fifo_set_overflow(state, 1);
		}
	}

	//return OK or not to write: 1 = OK, 0 = write is blocked
	return ret;
}

int fifo_is_writeblock(fifo_state_t * state){
	return ((state->o_flags & FIFO_FLAG_SET_WRITEBLOCK) != 0);
}

void fifo_set_writeblock(fifo_state_t * state, int value){
	if( value ){
		state->o_flags |= FIFO_FLAG_SET_WRITEBLOCK;
	} else {
		state->o_flags &= ~FIFO_FLAG_SET_WRITEBLOCK;
	}
}

int fifo_is_overflow(fifo_state_t * state){
	return ((state->o_flags & FIFO_FLAG_IS_OVERFLOW) != 0);
}

void fifo_set_overflow(fifo_state_t * state, int value){
	if( value ){
		state->o_flags |= FIFO_FLAG_IS_OVERFLOW;
	} else {
		state->o_flags &= ~FIFO_FLAG_IS_OVERFLOW;
	}
}

int fifo_read_buffer(const fifo_config_t * cfgp, fifo_state_t * state, char * buf){
	int i;
	u16 size = cfgp->size;
    for(i=0; i < state->read_len; i++){
		if ( state->head == state->tail ){ //check for data in the fifo buffer
			//there is no more data in the buffer
			break;
		} else {
			buf[i] = cfgp->buffer[state->tail];
			fifo_inc_tail(state, size);
		}
	}
	return i; //number of bytes read
}


int fifo_write_buffer(const fifo_config_t * cfgp, fifo_state_t * state, const char * buf, int non_blocking){
	int i;
	int size = cfgp->size;
	int writeblock = 1;
	if( non_blocking == 0 ){
		writeblock = fifo_is_writeblock(state);
	}
    for(i=0; i < state->write_len; i++){
		if( fifo_is_write_ok(state, size, writeblock) ){
			cfgp->buffer[state->head] = buf[i];
			fifo_inc_head(state, size);
		} else {
			break;
		}
	}
	return i; //number of bytes written
}

void fifo_flush(fifo_state_t * state){
	state->head = 0;
	state->tail = 0;
	fifo_set_overflow(state, 0);
}


void fifo_getinfo(fifo_info_t * info, const fifo_config_t * config, fifo_state_t * state){
	info->o_flags = FIFO_FLAG_IS_OVERFLOW;
	info->size = config->size - 1;
	if( state->head >= state->tail ){
		info->used = state->head - state->tail;
	} else {
		info->used = config->size - state->tail + state->head;
	}
	info->overflow = fifo_is_overflow(state);
	fifo_set_overflow(state, 0);
}

void fifo_data_received(const fifo_config_t * cfgp, fifo_state_t * state){
	int bytes_read;

    if( state->read_async != 0 ){
        state->read_async->nbyte = state->read_len; //update the number of bytes read??
        if( (bytes_read = fifo_read_buffer(cfgp, state, state->read_async->buf)) > 0 ){
			execute_read_callback(state, bytes_read, MCU_EVENT_FLAG_DATA_READY);
		}
	}
}

void fifo_cancel_rop(fifo_state_t * state){
	execute_read_callback(state, -1, MCU_EVENT_FLAG_CANCELED);
	//if( state->rop != 0 ){
	//	state->rop->nbyte = -1;
	//	mcu_execute_event_handler(&(state->rop->handler), MCU_EVENT_FLAG_CANCELED, 0);
	//	state->rop = 0;
	//}
}

void fifo_cancel_wop(fifo_state_t * state){
	execute_write_callback(state, -1, MCU_EVENT_FLAG_CANCELED);
	//if( state->rop != 0 ){
	//	state->rop->nbyte = -1;
	//	mcu_execute_event_handler(&(state->rop->handler), MCU_EVENT_FLAG_CANCELED, 0);
	//	state->rop = 0;
	//}
}

int fifo_data_transmitted(const fifo_config_t * cfgp, fifo_state_t * state){
	int bytes_written;
    if( state->write_async != NULL ){
        if( (bytes_written = fifo_write_buffer(cfgp, state, state->write_async->buf_const, 0)) > 0 ){
			execute_write_callback(state, bytes_written, MCU_EVENT_FLAG_WRITE_COMPLETE);
		}
	}

	return 1; //leave the callback in place
}

int fifo_open(const devfs_handle_t * handle){
    const fifo_config_t * config = handle->config;
    fifo_state_t * state = handle->state;
    return fifo_open_local(config, state);
}

int fifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    const fifo_config_t * config = handle->config;
    fifo_state_t * state = handle->state;
    return fifo_ioctl_local(config, state, request, ctl);
}

int fifo_write(const devfs_handle_t * handle, devfs_async_t * async){
    const fifo_config_t * config = handle->config;
    fifo_state_t * state = handle->state;
    return fifo_write_local(config, state, async, 1);
}

int fifo_read(const devfs_handle_t * handle, devfs_async_t * async){
    const fifo_config_t * config = handle->config;
    fifo_state_t * state = handle->state;
    return fifo_read_local(config, state, async, 1);
}

int fifo_close(const devfs_handle_t * handle){
    const fifo_config_t * config = handle->config;
    fifo_state_t * state = handle->state;
    return fifo_close_local(config, state);
}


int fifo_open_local(const fifo_config_t * config, fifo_state_t * state){
    return 0;
}

int fifo_close_local(const fifo_config_t * config, fifo_state_t * state){
    return 0;
}

int fifo_ioctl_local(const fifo_config_t * config, fifo_state_t * state, int request, void * ctl){
	fifo_attr_t * attr = ctl;
	mcu_action_t * action = ctl;
	switch(request){
	case I_FIFO_GETINFO:
        fifo_getinfo(ctl, config, state);
		return 0;
	case I_MCU_SETACTION:

		if( action->handler.callback == 0 ){

			if(action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
				fifo_cancel_wop(state);
			}

			if(action->o_events & MCU_EVENT_FLAG_DATA_READY ){
				fifo_cancel_wop(state);
			}

			return 0;
		}

		//fifo doesn't store a local handler so it can't set an arbitrary action
		errno = ENOTSUP;
		return -1;
	case I_FIFO_INIT:
        state->read_async = NULL;
        state->write_async = NULL;
		/* no break */
	case I_FIFO_FLUSH:
		fifo_flush(state);
        fifo_data_transmitted(config, state); //something might be waiting to write the fifo
		return 0;
	case I_FIFO_SETATTR:
		if( attr->o_flags & FIFO_FLAG_SET_WRITEBLOCK ){
			fifo_set_writeblock(state, 1);
			if( attr->o_flags & FIFO_FLAG_IS_OVERFLOW ){
				fifo_set_writeblock(state, 0);
			}
		}


		if( fifo_is_writeblock(state) ){
			//make sure the FIFO is not currently blocked
            fifo_data_transmitted(config, state); //something might be waiting to write the fifo
		}
		return 0;
	}
	errno = EINVAL;
	return -1;
}

int fifo_read_local(const fifo_config_t * config, fifo_state_t * state, devfs_async_t * async, int allow_callback){
	int bytes_read;

    if ( state->read_async != NULL ){
		errno = EBUSY; //the device is temporarily unavailable
		return -1;
	}

    state->read_len = async->nbyte;
    bytes_read = fifo_read_buffer(config, state, async->buf); //see if there are bytes in the buffer
	if ( bytes_read == 0 ){
        if( (async->flags & O_NONBLOCK) ){
			errno = EAGAIN;
			bytes_read = -1;
		} else {
            state->read_async = async;
            state->read_len = async->nbyte; //total number of bytes to read
            async->nbyte = 0; //number of bytes read so far
		}
    } else if( (bytes_read > 0) && allow_callback ){
		//see if anything needs to write the FIFO
        fifo_data_transmitted(config, state);
	}

	return bytes_read;
}

int fifo_write_local(const fifo_config_t * config, fifo_state_t * state, devfs_async_t * async, int allow_callback){
	int bytes_written;
	int non_blocking;

    if ( state->write_async != NULL ){
        async->nbyte = EBUSY;
		return -1; //caller will block until FIFO is ready to write
	}

    state->write_len = async->nbyte;
    non_blocking = ((async->flags & O_NONBLOCK) != 0);
    bytes_written = fifo_write_buffer(config, state, async->buf_const, non_blocking); //see if there are bytes in the buffer
	if ( bytes_written == 0 ){
		if( non_blocking ){
			errno = EAGAIN;
			bytes_written = -1;
		} else {
            state->write_async = async;
            state->write_len = async->nbyte;
            async->nbyte = 0;
		}
	}

    if( (bytes_written > 0) && allow_callback ){
        fifo_data_received(config, state);
	}

	return bytes_written;
}
