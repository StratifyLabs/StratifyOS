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
#include "iface/dev/ffifo.h"
#include "dev/ffifo.h"


char * ffifo_get_frame(const ffifo_cfg_t * cfgp, u16 frame){
	return cfgp->buffer + frame * cfgp->frame_size;
}

void ffifo_inc_tail(ffifo_state_t * state, u16 count){
	state->tail++;
	if ( state->tail == count ){
		state->tail = 0;
	}
}

void ffifo_inc_head(ffifo_state_t * state, u16 count){
	state->head++;
	if ( state->head == count ){
		state->head = 0;
	}

	if ( state->head == state->tail ){
		state->tail++;
		if ( state->tail == count ){
			state->tail = 0;
		}
		ffifo_set_overflow(state, 1);
	}
}

int ffifo_is_write_ok(ffifo_state_t * state, u16 count, int writeblock){
	int ret = 1;
	if( ((state->tail == 0) && (state->head == count-1)) ||
			((state->head) + 1 == state->tail)
	){
		if( writeblock ){
			//cannot write anymore data at this time
			ret = 0;
		} else {
			//OK to write but it will cause an overflow
			ffifo_inc_tail(state, count);
			ffifo_set_overflow(state, 1);
		}
	}

	//return OK or not to write: 1 = OK, 0 = write is blocked
	return ret;
}

int ffifo_is_writeblock(ffifo_state_t * state){
	return ((state->o_flags & FIFO_FLAGS_WRITEBLOCK) != 0);
}

void ffifo_set_writeblock(ffifo_state_t * state, int value){
	if( value ){
		state->o_flags |= FIFO_FLAGS_WRITEBLOCK;
	} else {
		state->o_flags &= ~FIFO_FLAGS_WRITEBLOCK;
	}
}

int ffifo_is_overflow(ffifo_state_t * state){
	return ((state->o_flags & FIFO_FLAGS_OVERFLOW) != 0);
}

void ffifo_set_overflow(ffifo_state_t * state, int value){
	if( value ){
		state->o_flags |= FIFO_FLAGS_OVERFLOW;
	} else {
		state->o_flags &= ~FIFO_FLAGS_OVERFLOW;
	}
}

int ffifo_read_buffer(const ffifo_cfg_t * cfgp, ffifo_state_t * state, char * buf){
	int i;
	u16 count = cfgp->count;
	u16 frame_size = cfgp->frame_size;
	for(i=0; i < state->rop_len; i += frame_size){
		if ( state->head == state->tail ){
			//there is no more data in the buffer to read
			break;
		} else {
			memcpy(buf, ffifo_get_frame(cfgp, state->tail), frame_size);
			buf += frame_size;
			ffifo_inc_tail(state, count);
		}
	}
	return i; //number of bytes read
}


int ffifo_write_buffer(const ffifo_cfg_t * cfgp, ffifo_state_t * state, const char * buf){
	int i;
	u16 count = cfgp->count;
	u16 frame_size = cfgp->frame_size;
	int writeblock = ffifo_is_writeblock(state);
	for(i=0; i < state->wop_len; i+=frame_size){
		if( ffifo_is_write_ok(state, count, writeblock) ){
			memcpy( ffifo_get_frame(cfgp, state->head), buf, frame_size);
			buf += frame_size;
			ffifo_inc_head(state, count);
		} else {
			break;
		}
	}
	return i; //number of frames written
}

void ffifo_flush(ffifo_state_t * state){
	state->head = 0;
	state->tail = 0;
	ffifo_set_overflow(state, 0);
}

void ffifo_getattr(ffifo_attr_t * attr, const ffifo_cfg_t * cfgp, ffifo_state_t * state){
	attr->count = cfgp->count;
	attr->frame_size = cfgp->frame_size;

	if( state->head >= state->tail ){
		attr->used = state->head - state->tail;
	} else {
		attr->used = attr->count - state->tail + state->head;
	}

	attr->o_flags = state->o_flags;
	//clear the overflow flags after it is read
	ffifo_set_overflow(state, 0);
}


void ffifo_data_received(const ffifo_cfg_t * cfgp, ffifo_state_t * state){
	int bytes_read;

	if( state->rop != NULL ){
		state->rop->nbyte = state->rop_len; //update the number of bytes read??
		if( (bytes_read = ffifo_read_buffer(cfgp, state, state->rop->buf)) > 0 ){
			state->rop->nbyte = bytes_read;
			if ( state->rop->callback(state->rop->context, (mcu_event_t)NULL) == 0 ){
				state->rop = NULL;
			}
		}
	}
}

void ffifo_cancel_rop(ffifo_state_t * state){
	if( state->rop != NULL ){
		state->rop->nbyte = -1;
		if ( state->rop->callback(state->rop->context, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED)) == 0 ){
			state->rop = NULL;
		}
	}
}


void ffifo_data_transmitted(const ffifo_cfg_t * cfgp, ffifo_state_t * state){
	int bytes_written;
	if( state->wop != NULL ){
		if( (bytes_written = ffifo_write_buffer(cfgp, state, state->wop->cbuf)) > 0 ){
			state->wop->nbyte = bytes_written;
			if ( state->wop->callback(state->wop->context, NULL) == 0 ){
				state->wop = NULL;
			}
		}
	}
}




int ffifo_open(const device_cfg_t * cfg){
	//const ffifo_cfg_t * cfgp = cfg->dcfg;
	//ffifo_state_t * state = cfg->state;
	return 0;
}

int ffifo_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	ffifo_attr_t * attr = ctl;
	const ffifo_cfg_t * cfgp = cfg->dcfg;
	ffifo_state_t * state = cfg->state;
	switch(request){
	case I_FFIFO_GETATTR:
		ffifo_getattr(attr, cfgp, state);
		return 0;
	case I_FFIFO_INIT:
		state->rop = NULL;
		state->wop = NULL;
		/* no break */
	case I_FFIFO_FLUSH:
		ffifo_flush(state);
		ffifo_data_transmitted(cfgp, state); //something might be waiting to write the fifo
		return 0;
	case I_FFIFO_SETWRITEBLOCK:
		ffifo_set_writeblock(state, (int)ctl);
		if( ffifo_is_writeblock(state) ){
			//make sure the FIFO is not currently blocked
			ffifo_data_transmitted(cfgp, state);
		}
		return 0;
	}
	errno = EINVAL;
	return -1;
}



int ffifo_read_local(const ffifo_cfg_t * cfgp, ffifo_state_t * state, device_transfer_t * rop){
	int bytes_read;

	if ( state->rop != NULL ){
		errno = EAGAIN; //the device is temporarily unavailable
		return -1;
	}

	//writes need to be a integer multiple of the frame size
	if( (rop->nbyte % cfgp->frame_size) != 0 ){
		errno = EINVAL;
		return -1;
	}

	state->rop_len = rop->nbyte;
	bytes_read = ffifo_read_buffer(cfgp, state, rop->buf); //see if there are bytes in the buffer
	if ( bytes_read == 0 ){
		if( (rop->flags & O_NONBLOCK) ){
			errno = EAGAIN;
			bytes_read = -1;
		} else {
			state->rop = rop;
			state->rop_len = rop->nbyte; //total number of bytes to read
			rop->nbyte = 0; //number of bytes read so far
		}
	}

	return bytes_read;
}

int ffifo_read(const device_cfg_t * cfg, device_transfer_t * rop){
	const ffifo_cfg_t * cfgp = cfg->dcfg;
	ffifo_state_t * state = cfg->state;
	int bytes_read;

	bytes_read = ffifo_read_local(cfgp, state, rop);

	if( bytes_read > 0 ){
		//see if anything needs to write the FIFO
		ffifo_data_transmitted(cfgp, state);

		if( cfgp->notify_on_read ){
			cfgp->notify_on_read(bytes_read);
		}
	}

	return bytes_read;
}

int ffifo_write_local(const ffifo_cfg_t * cfgp, ffifo_state_t * state, device_transfer_t * wop){
	int bytes_written;

	if ( state->wop != NULL ){
		wop->nbyte = EAGAIN;
		return -1; //caller can try again
	}

	//writes need to be a integer multiple of the frame size
	if( (wop->nbyte % cfgp->frame_size) != 0 ){
		errno = EINVAL;
		return -1;
	}

	state->wop_len = wop->nbyte;
	bytes_written = ffifo_write_buffer(cfgp, state, wop->cbuf); //see if there are bytes in the buffer
	if ( bytes_written == 0 ){
		if( wop->flags & O_NONBLOCK ){
			errno = EAGAIN;
			bytes_written = -1;
		} else {
			state->wop = wop;
			state->wop_len = wop->nbyte;
			wop->nbyte = 0;
		}
	} else if( (bytes_written > 0) && (cfgp->notify_on_write != 0) ){
		cfgp->notify_on_write(bytes_written);
	}

	return bytes_written;
}

int ffifo_write(const device_cfg_t * cfg, device_transfer_t * wop){
	const ffifo_cfg_t * cfgp = cfg->dcfg;
	ffifo_state_t * state = cfg->state;
	int bytes_written;

	bytes_written = ffifo_write_local(cfgp, state, wop);
	if( bytes_written > 0 ){
		ffifo_data_received(cfgp, state);
	}

	return bytes_written;
}

int ffifo_close(const device_cfg_t * cfg){
	return 0;
}

