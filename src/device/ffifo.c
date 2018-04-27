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
#include "sos/dev/ffifo.h"
#include "device/ffifo.h"


static void execute_write_callback(ffifo_state_t * state, int nbyte, u32 o_events);
static void execute_read_callback(ffifo_state_t * state, int nbyte, u32 o_events);


void execute_read_callback(ffifo_state_t * state, int nbyte, u32 o_events){
    if( state->read_async != NULL ){
        state->read_async->nbyte = nbyte;
        mcu_execute_event_handler(&(state->read_async->handler), o_events, 0);
        if( state->read_async->handler.callback == 0 ){
            state->read_async = 0;
        }
    }
}

void execute_write_callback(ffifo_state_t * state, int nbyte, u32 o_events){
    if( state->write_async != NULL ){
        state->write_async->nbyte = nbyte;
        mcu_execute_event_handler(&(state->write_async->handler), o_events, 0);
        if( state->write_async->handler.callback == 0 ){
            state->write_async = 0;
        }
    }
}

char * ffifo_get_frame(const ffifo_config_t * cfgp, u16 frame){
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

int ffifo_read_buffer(const ffifo_config_t * cfgp, ffifo_state_t * state, char * buf, int len){
    int i;
    u16 count = cfgp->count;
    u16 frame_size = cfgp->frame_size;
    for(i=0; i < len; i += frame_size){
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


int ffifo_write_buffer(const ffifo_config_t * cfgp, ffifo_state_t * state, const char * buf, int len){
    int i;
    u16 count = cfgp->count;
    u16 frame_size = cfgp->frame_size;
    int writeblock = ffifo_is_writeblock(state);
    for(i=0; i < len; i+=frame_size){
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

void ffifo_getinfo(ffifo_info_t * info, const ffifo_config_t * config, ffifo_state_t * state){
    info->count = config->count;
    info->frame_size = config->frame_size;

    if( state->head >= state->tail ){
        info->used = state->head - state->tail;
    } else {
        info->used = info->count - state->tail + state->head;
    }

    info->o_flags = state->o_flags;
    //clear the overflow flags after it is read
    ffifo_set_overflow(state, 0);
}


void ffifo_data_received(const ffifo_config_t * handle, ffifo_state_t * state){
    int bytes_read;
    if( state->read_async != NULL ){
        if( (bytes_read = ffifo_read_buffer(handle, state, state->read_async->buf, state->read_len)) > 0 ){
            execute_read_callback(state, bytes_read, MCU_EVENT_FLAG_DATA_READY);
        }
    }
}

void ffifo_cancel_rop(ffifo_state_t * state){
    execute_read_callback(state, -1, MCU_EVENT_FLAG_CANCELED);
}


void ffifo_cancel_wop(ffifo_state_t * state){
    execute_write_callback(state, -1, MCU_EVENT_FLAG_CANCELED);
}



void ffifo_data_transmitted(const ffifo_config_t * config, ffifo_state_t * state){
    int bytes_written;
    if( state->write_async != NULL ){
        if( (bytes_written = ffifo_write_buffer(config, state, state->write_async->buf_const, state->write_len)) > 0 ){
            execute_write_callback(state, bytes_written, MCU_EVENT_FLAG_WRITE_COMPLETE);
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
                ffifo_cancel_wop(state);
            }

            if( action->o_events & MCU_EVENT_FLAG_DATA_READY){
                //cancel any ongoing operations
                ffifo_cancel_rop(state);
            }
            return 0;
        }

        //ffifo doesn't store a local handler so it can't set an arbitrary action on read/write
        return SYSFS_SET_RETURN(ENOTSUP);
    case I_FFIFO_GETINFO:
        ffifo_getinfo(info, config, state);
        return 0;
    case I_FFIFO_INIT:
        state->read_async = NULL;
        state->write_async = NULL;
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

    if ( state->read_async != NULL ){
        return SYSFS_SET_RETURN(EBUSY);
    }

    //reads need to be a integer multiple of the frame size
    if( (async->nbyte % config->frame_size) != 0 ){
        return SYSFS_SET_RETURN(EINVAL);
    }

    bytes_read = ffifo_read_buffer(config, state, async->buf, async->nbyte); //see if there are bytes in the buffer
    if ( bytes_read == 0 ){
        if( (async->flags & O_NONBLOCK) ){
            bytes_read = SYSFS_SET_RETURN(EAGAIN);
        } else {
            state->read_async = async;
            state->read_len = async->nbyte; //total number of bytes to read
            async->nbyte = 0; //number of bytes read so far
        }
    } else if( (bytes_read > 0) && allow_callback ){
        //see if anything needs to write the FIFO
        ffifo_data_transmitted(config, state);
    }

    return bytes_read;
}



int ffifo_write_local(const ffifo_config_t * config, ffifo_state_t * state, devfs_async_t * async, int allow_callback){
    int bytes_written;

    if ( state->write_async != NULL ){
        return SYSFS_SET_RETURN(EBUSY);
    }

    //writes need to be a integer multiple of the frame size
    if( (async->nbyte % config->frame_size) != 0 ){
        return SYSFS_SET_RETURN(EINVAL);
    }

    bytes_written = ffifo_write_buffer(config, state, async->buf_const, async->nbyte); //see if there are bytes in the buffer
    if ( bytes_written == 0 ){
        if( async->flags & O_NONBLOCK ){
            bytes_written = SYSFS_SET_RETURN(EAGAIN);
        } else {
            state->write_async = async;
            state->write_len = async->nbyte;
            async->nbyte = 0;
        }
    } else if( (bytes_written > 0) && allow_callback ){
        ffifo_data_received(config, state);
    }

    return bytes_written;
}

