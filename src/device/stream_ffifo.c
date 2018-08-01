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


#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "mcu/debug.h"
#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "device/stream_ffifo.h"

static int event_write_complete(void * context, const mcu_event_t * event);
static int event_data_ready(void * context, const mcu_event_t * event);

int event_write_complete(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;
    int nbyte;
    ffifo_state_t * ffifo_state = &state->tx.ffifo;

    if(state->tx.async.nbyte < 0){
        state->tx.error = state->tx.async.nbyte;
        return 0;
    }

    nbyte = state->tx.async.nbyte/2;

    for(u32 frames = 0; frames < config->tx.count/2; frames++){
        if( ffifo_state->atomic_position.access.head == ffifo_state->atomic_position.access.tail ){
            //no data to read -- send a zero frame -- this isn't quite right
            memset(state->tx.async.buf, 0, nbyte);
        } else {
            state->tx.count++;

            //increment the tail for the frame that was written
            if( ffifo_state->atomic_position.access.tail == config->tx.count ){
                ffifo_state->atomic_position.access.tail = ffifo_state->atomic_position.access.head;
            }
            ffifo_inc_tail(ffifo_state, config->tx.count);
        }

        //execute the FFIFO action if something is trying to write the ffifo
        ffifo_data_transmitted(&config->tx, &state->tx.ffifo);
    }

    return 1;
}

int event_data_ready(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;
    ffifo_state_t * ffifo_state = &state->rx.ffifo;
    u32 o_events = event->o_events;


    //check for errors or abort
    if( (state->rx.async.nbyte < 0) || (o_events & MCU_EVENT_FLAG_CANCELED) ){
        state->rx.error = state->rx.async.nbyte;
        mcu_debug_log_error(MCU_DEBUG_DEVICE, "%s:%d", __FUNCTION__, __LINE__);
        //tell ffifo there won't be any data coming?
        return 0;
    }

    //check for overflow
    if( ffifo_state->atomic_position.access.tail == config->rx.count ){
        if( ffifo_state->o_flags & FIFO_FLAG_IS_READ_BUSY ){
            ffifo_state->o_flags |= FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY;
        }
        ffifo_state->o_flags |= FIFO_FLAG_IS_OVERFLOW;
    }

    //increment the head for the frame received
    for(u32 frames = 0; frames < config->rx.count/2; frames++){
        ffifo_inc_head(&state->rx.ffifo, config->rx.count);
        state->rx.count++;
        ffifo_data_received(&config->rx, &state->rx.ffifo);
    }

    return 1;
}

int stream_ffifo_open(const devfs_handle_t * handle){
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;
    int ret;

    ret = config->device->driver.open(&config->device->handle);
    if( ret <  0 ){ return ret; }

    //is device a streaming device? --does it support MCU_EVENT_FLAG_HIGH and MCU_EVENT_FLAG_LOW

    ret = ffifo_open_local(&config->tx, &state->tx.ffifo);
    if( ret < 0 ){ return ret;}

    return ffifo_open_local(&config->rx, &state->rx.ffifo);
}

int stream_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;
    stream_ffifo_info_t * info = ctl;
    const stream_ffifo_attr_t * attr = ctl;
    mcu_action_t * action = ctl;

    switch(request){

    case I_STREAM_FFIFO_GETVERSION: return STREAM_FFIFO_VERSION;
    case I_STREAM_FFIFO_SETATTR:

        if( attr->o_flags & STREAM_FFIFO_FLAG_START ){

            mcu_debug_log_info(MCU_DEBUG_DEVICE, "Start Stream");

            if( config->tx.buffer ){
                state->tx.async.loc = 0;
                state->tx.async.tid = task_get_current();
                state->tx.async.flags = O_RDWR;
                state->tx.async.buf = config->tx.buffer;
                state->tx.async.nbyte = config->tx.frame_size * config->tx.count;
                state->tx.async.handler.context = (void*)handle;
                state->tx.async.handler.callback = event_write_complete;

                //write block is true but the function doesn't block -- returns EGAIN if it is written while full
                ffifo_set_writeblock(&(state->tx.ffifo), 1);

                state->tx.count = 0;
                state->tx.error = 0;

                state->tx.ffifo.transfer_handler.read = 0;
                state->tx.ffifo.transfer_handler.write = 0;
                ffifo_flush(&(state->tx.ffifo));

                //ffifo is empty so the head must be incremented on start
                ffifo_inc_head(&state->tx.ffifo, config->tx.count);
                memset(state->tx.async.buf, 0, state->tx.async.nbyte);

                //start writing data to the I2S -- zeros are written if there is no data in the fifo
                if( config->device->driver.write(&config->device->handle, &(state->tx.async)) < 0 ){
                    return SYSFS_SET_RETURN(EIO);
                }
            }


            if( config->rx.buffer ){

                //the driver writes the I2S RX FIFO that is read from the I2S
                state->rx.async.loc = 0;
                state->rx.async.tid = task_get_current();
                state->rx.async.flags = O_RDWR;
                state->rx.async.buf = config->rx.buffer;
                state->rx.async.nbyte = config->rx.frame_size * config->rx.count;
                state->rx.async.handler.context = (void*)handle;
                state->rx.async.handler.callback = event_data_ready;

                state->rx.error = 0;
                state->rx.count = 0;

                state->rx.ffifo.transfer_handler.read = 0;
                state->rx.ffifo.transfer_handler.write = 0;
                ffifo_flush(&(state->rx.ffifo));

                //on the first call the FIFO is full of zeros and returns immediately
                if( config->device->driver.read(&config->device->handle, &(state->rx.async)) < 0 ){
                    return SYSFS_SET_RETURN(EIO);
                }
            }
        } else if( attr->o_flags & STREAM_FFIFO_FLAG_STOP ){


        }

        if( attr->o_flags & STREAM_FFIFO_FLAG_FLUSH ){
            ffifo_flush(&(state->tx.ffifo));
            ffifo_flush(&(state->rx.ffifo));

            state->rx.async.buf = config->rx.buffer;
            state->rx.async.nbyte = config->rx.frame_size;

            state->tx.async.buf = config->tx.buffer;
            state->tx.async.nbyte = config->tx.frame_size;
        }


        return 0;

    case I_STREAM_FFIFO_GETINFO:
        ffifo_getinfo(&(info->tx.ffifo), &(config->tx), &(state->tx.ffifo));
        ffifo_getinfo(&(info->rx.ffifo), &(config->rx), &(state->rx.ffifo));
        info->tx.count = state->tx.count;
        info->rx.count = state->rx.count;
        info->tx.error = state->tx.error;
        info->rx.error = state->rx.error;
        return 0;

    case I_STREAM_FFIFO_SETACTION:
    case I_MCU_SETACTION:

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_DATA_READY) && (action->handler.callback == 0) ){
            //cancel the ffifo rx
            ffifo_cancel_async_read(&(state->rx.ffifo));
        }

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE) && (action->handler.callback == 0) ){
            //cancel the ffifo rx
            ffifo_cancel_async_write(&(state->tx.ffifo));
        }
        /* no break */
    }

    return config->device->driver.ioctl(&config->device->handle, request, ctl);
}

int stream_ffifo_read(const devfs_handle_t * handle, devfs_async_t * async){
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;

    if( async->nbyte % config->rx.frame_size != 0 ){
        return SYSFS_SET_RETURN(EINVAL);
    }

    //this will never need to execute a callback because the FIFO is written by hardware
    return ffifo_read_local(&(config->rx), &(state->rx.ffifo), async, 0);
}

int stream_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async){
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;

    if( async->nbyte % config->tx.frame_size != 0 ){
        return SYSFS_SET_RETURN(EINVAL);
    }

    //this will never need to execute a callback because the FIFO is read by hardware
    return ffifo_write_local(&(config->tx), &(state->tx.ffifo), async, 0);
}

int stream_ffifo_close(const devfs_handle_t * handle){
    const stream_ffifo_config_t * config = handle->config;
    stream_ffifo_state_t * state = handle->state;
    ffifo_close_local(&(config->tx), &(state->tx.ffifo));
    ffifo_close_local(&(config->rx), &(state->rx.ffifo));
    return config->device->driver.close(&config->device->handle);
}


