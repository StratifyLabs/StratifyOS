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
#include "cortexm/cortexm.h"
#include "mcu/debug.h"
#include "sos/dev/ffifo.h"
#include "device/ffifo.h"

char * ffifo_get_frame(const ffifo_config_t * cfgp, u16 frame){
	return cfgp->buffer + (frame * cfgp->frame_size);
}

void ffifo_inc_tail(ffifo_state_t * state, u16 count){
	state->atomic_position.access.tail++;
	if ( state->atomic_position.access.tail == count ){
		state->atomic_position.access.tail = 0;
	}
}

void ffifo_inc_head(ffifo_state_t * state, u16 count){
	state->atomic_position.access.head++;
	if ( state->atomic_position.access.head == count ){
		state->atomic_position.access.head = 0;
	}

	//this marks the ffifo as full by setting tail to count
	if( state->atomic_position.access.head == state->atomic_position.access.tail ){
		state->atomic_position.access.tail = count;
	}

}

int ffifo_is_write_ok(ffifo_state_t * state, u16 count, int writeblock){
	int ret = 1;
	fifo_atomic_position_t atomic_postion;
	atomic_postion.atomic_access = state->atomic_position.atomic_access;
	if( atomic_postion.access.tail == count ){
		if( writeblock ){
			//cannot write anymore data at this time
			ret = 0;
		} else {
			//OK to write but it will cause an overflow
			if( state->o_flags & FIFO_FLAG_IS_READ_BUSY ){
				state->o_flags |= FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY;
			}
			ffifo_set_overflow(state, 1);
		}
	}

	//return OK or not to write: 1 = OK, 0 = write is blocked
	return ret;
}

int ffifo_is_writeblock(ffifo_state_t * state){
	return ((state->o_flags & FIFO_FLAG_SET_WRITEBLOCK) != 0);
}

void ffifo_set_writeblock(ffifo_state_t * state, int value){
	if( value ){
		state->o_flags |= FIFO_FLAG_SET_WRITEBLOCK;
	} else {
		state->o_flags &= ~FIFO_FLAG_SET_WRITEBLOCK;
	}
}

int ffifo_is_overflow(ffifo_state_t * state){
	return ((state->o_flags & FIFO_FLAG_IS_OVERFLOW) != 0);
}

void ffifo_set_overflow(ffifo_state_t * state, int value){
	if( value ){
		state->o_flags |= FIFO_FLAG_IS_OVERFLOW;
	} else {
		state->o_flags &= ~FIFO_FLAG_IS_OVERFLOW;
	}
}

void * ffifo_get_head(const ffifo_config_t * config, ffifo_state_t * state){
	return config->buffer + (state->atomic_position.access.head * config->frame_size);
}

void * ffifo_get_tail(const ffifo_config_t * config, ffifo_state_t * state){
	return config->buffer + (state->atomic_position.access.tail * config->frame_size);
}


int ffifo_read_buffer(const ffifo_config_t * config, ffifo_state_t * state, char * buf, int len){
	int i;
	u16 count = config->frame_count;
	u16 frame_size = config->frame_size;
	char * frame;
	fifo_atomic_position_t atomic_position;
	int read_was_clobbered = 0;
	for(i=0; i < len; i += frame_size){

		state->o_flags |= FIFO_FLAG_IS_READ_BUSY;
		atomic_position.atomic_access = state->atomic_position.atomic_access;

		if ( atomic_position.access.head != atomic_position.access.tail ){

			if( atomic_position.access.tail == count ){
				atomic_position.access.tail = atomic_position.access.head;
			}

			frame = ffifo_get_frame(config, atomic_position.access.tail);
			memcpy(buf, frame, frame_size);
			atomic_position.access.tail++;
			if( atomic_position.access.tail == count ){
				atomic_position.access.tail = 0;
			}

			//an interrupt here before the tail is assigned will cause a problem
			state->atomic_position.access.tail = atomic_position.access.tail;
			//an interrupt here is OK because the write can write to the open spot
			if( state->o_flags & FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY ){
				read_was_clobbered = 1;
				//if the read was clobbered the buffer is full
				state->atomic_position.access.tail = frame_size;
			}
			state->o_flags &= ~(FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY|FIFO_FLAG_IS_READ_BUSY);

			if( read_was_clobbered ){
				return i;
			}

			buf += frame_size;
		} else {
			state->o_flags &= ~(FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY|FIFO_FLAG_IS_READ_BUSY);
			break;
		}

	}
	return i; //number of bytes read
}


int ffifo_write_buffer(const ffifo_config_t * config, ffifo_state_t * state, const char * buf, int len){
	int i;
	u16 frame_count = config->frame_count;
	u16 frame_size = config->frame_size;
	void * frame;
	int writeblock = ffifo_is_writeblock(state);
	for(i=0; i < len; i+=frame_size){
		if( ffifo_is_write_ok(state, frame_count, writeblock) ){
			state->o_flags	|= FIFO_FLAG_IS_WRITE_BUSY;
			frame = ffifo_get_frame(config, state->atomic_position.access.head);
			memcpy(frame, buf, frame_size);

			//don't inc head until the data is copied
			ffifo_inc_head(state, frame_count);
			buf += frame_size;
			state->o_flags	&= ~FIFO_FLAG_IS_WRITE_BUSY;

		} else {
			break;
		}
	}
	return i; //number of frames written
}

void ffifo_flush(ffifo_state_t * state){
	state->atomic_position.atomic_access = 0;
	ffifo_set_overflow(state, 0);
}

int ffifo_getinfo(ffifo_info_t * info, const ffifo_config_t * config, ffifo_state_t * state){
	info->o_flags = FIFO_FLAG_SET_WRITEBLOCK | FIFO_FLAG_IS_OVERFLOW;
	info->frame_count = config->frame_count;
	info->frame_size = config->frame_size;
	fifo_atomic_position_t atomic_position;
	atomic_position.atomic_access = state->atomic_position.atomic_access;

	if( atomic_position.access.tail == config->frame_count ){
		info->frame_count_ready = info->frame_count;
	} else if( atomic_position.access.head >= atomic_position.access.tail ){
		info->frame_count_ready = atomic_position.access.head - atomic_position.access.tail;
	} else {
		info->frame_count_ready = info->frame_count - atomic_position.access.tail + atomic_position.access.head;
	}

	info->o_flags = state->o_flags;
	state->o_flags &= ~FIFO_FLAG_IS_OVERFLOW;
	return 0;
}

void ffifo_data_received(const ffifo_config_t * handle, ffifo_state_t * state){
	int bytes_read;
	if( state->transfer_handler.read != NULL ){
		if( (bytes_read = ffifo_read_buffer(
				  handle,
				  state,
				  state->transfer_handler.read->buf,
				  state->transfer_handler.read->nbyte
				  )) > 0 ){
			devfs_execute_read_handler(
						&state->transfer_handler,
						0,
						bytes_read,
						MCU_EVENT_FLAG_DATA_READY
						);
		}
	}
}

void ffifo_cancel_async_read(ffifo_state_t * state){
	devfs_execute_read_handler(
				&state->transfer_handler,
				0,
				SYSFS_SET_RETURN(EIO),
				MCU_EVENT_FLAG_CANCELED);
}


void ffifo_cancel_async_write(ffifo_state_t * state){
	devfs_execute_write_handler(
				&state->transfer_handler,
				0,
				SYSFS_SET_RETURN(EIO),
				MCU_EVENT_FLAG_CANCELED);
}



void ffifo_data_transmitted(const ffifo_config_t * config, ffifo_state_t * state){
	int bytes_written;
	if( state->transfer_handler.write != NULL ){
		if( (bytes_written = ffifo_write_buffer(config, state, state->transfer_handler.write->buf_const, state->transfer_handler.write->nbyte)) > 0 ){
			devfs_execute_write_handler(
						&state->transfer_handler,
						0,
						bytes_written,
						MCU_EVENT_FLAG_WRITE_COMPLETE);
		}
	}
}




int ffifo_open(const devfs_handle_t * handle){
	const ffifo_config_t * cfgp = handle->config;
	ffifo_state_t * state = handle->state;
	return ffifo_open_local(cfgp, state);
}

int ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	const ffifo_config_t * config = handle->config;
	ffifo_state_t * state = handle->state;
	return ffifo_ioctl_local(config, state, request, ctl);
}

int ffifo_read(const devfs_handle_t * handle, devfs_async_t * rop){
	const ffifo_config_t * config = handle->config;
	ffifo_state_t * state = handle->state;
	return ffifo_read_local(config, state, rop, 1);
}

int ffifo_write(const devfs_handle_t * handle, devfs_async_t * wop){
	const ffifo_config_t * config = handle->config;
	ffifo_state_t * state = handle->state;
	return ffifo_write_local(config, state, wop, 1);
}

int ffifo_close(const devfs_handle_t * handle){
	const ffifo_config_t * cfgp = handle->config;
	ffifo_state_t * state = handle->state;
	return ffifo_close_local(cfgp, state);
}

int ffifo_open_local(const ffifo_config_t * config, ffifo_state_t * state){
	return 0;
}

int ffifo_close_local(const ffifo_config_t * config, ffifo_state_t * state){
	return 0;
}

int ffifo_ioctl_local(const ffifo_config_t * config, ffifo_state_t * state, int request, void * ctl){
	ffifo_attr_t * attr = ctl;
	ffifo_info_t * info = ctl;
	mcu_action_t * action = ctl;
	switch(request){
		case I_MCU_SETACTION:
			if( action->handler.callback == 0 ){
				if( action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
					//cancel any ongoing operations
					ffifo_cancel_async_write(state);
				}

				if( action->o_events & MCU_EVENT_FLAG_DATA_READY){
					//cancel any ongoing operations
					ffifo_cancel_async_read(state);
				}
				return 0;
			}

			//ffifo doesn't store a local handler so it can't set an arbitrary action on read/write
			return SYSFS_SET_RETURN(ENOTSUP);
		case I_FFIFO_GETINFO:
			ffifo_getinfo(info, config, state);
			return 0;
		case I_FFIFO_INIT:
			state->transfer_handler.read = NULL;
			state->transfer_handler.write = NULL;
			/* no break */
		case I_FFIFO_FLUSH:
			ffifo_flush(state);
			ffifo_data_transmitted(config, state); //something might be waiting to write the fifo
			return 0;
		case I_FFIFO_SETATTR:
			if( attr->o_flags & FIFO_FLAG_SET_WRITEBLOCK ){
				ffifo_set_writeblock(state, 1);
			} else if( attr->o_flags & FIFO_FLAG_IS_OVERFLOW ){
				ffifo_set_writeblock(state, 0);
			}
			if( ffifo_is_writeblock(state) ){
				//make sure the FIFO is not currently blocked
				ffifo_data_transmitted(config, state);
			}
			return 0;
	}
	return SYSFS_SET_RETURN(EINVAL);
}


int ffifo_read_local(const ffifo_config_t * config, ffifo_state_t * state, devfs_async_t * async, int allow_callback){
	int bytes_read;

	DEVFS_DRIVER_IS_BUSY(state->transfer_handler.read, async);

	//reads need to be a integer multiple of the frame size
	if( (async->nbyte % config->frame_size) != 0 ){
		bytes_read = SYSFS_SET_RETURN(EINVAL);
	} else {

		bytes_read = ffifo_read_buffer(config, state, async->buf, async->nbyte); //see if there are bytes in the buffer
		if ( bytes_read == 0 ){
			if( (async->flags & O_NONBLOCK) || (state->atomic_position.access.tail == config->frame_count) ){
				bytes_read = SYSFS_SET_RETURN(EAGAIN);
			}
		} else if( (bytes_read > 0) && allow_callback ){
			//see if anything needs to write the FIFO
			ffifo_data_transmitted(config, state);
		}
	}

	if( bytes_read != 0 ){
		state->transfer_handler.read = 0;
	}

	return bytes_read;
}



int ffifo_write_local(const ffifo_config_t * config, ffifo_state_t * state, devfs_async_t * async, int allow_callback){
	int bytes_written;

	DEVFS_DRIVER_IS_BUSY(state->transfer_handler.write, async);

	//writes need to be a integer multiple of the frame size
	if( (async->nbyte % config->frame_size) != 0 ){
		bytes_written = SYSFS_SET_RETURN(EINVAL);
	} else {
		bytes_written = ffifo_write_buffer(config, state, async->buf_const, async->nbyte); //see if there are bytes in the buffer
		if ( bytes_written == 0 ){
			if( async->flags & O_NONBLOCK ){
				bytes_written = SYSFS_SET_RETURN(EAGAIN);
			}
		} else if( (bytes_written > 0) && allow_callback ){
			ffifo_data_received(config, state);
		}
	}

	if( bytes_written != 0 ){
		state->transfer_handler.write = 0;
	}

	return bytes_written;
}

