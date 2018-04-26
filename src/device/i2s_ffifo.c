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


#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "device/i2s_ffifo.h"

static int i2s_event_write_complete(void * context, const mcu_event_t * event);
static int i2s_event_data_ready(void * context, const mcu_event_t * event);

int i2s_event_write_complete(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;
    int ret;

    //now if there is data available, copy it to the outgoing frame buffer
    if( (ret = ffifo_read_buffer(&(config->tx.ffifo),
                                 &(state->tx.ffifo),
                                 config->tx.frame_buffer,
                                 config->frame_buffer_size)) <= 0 ){
        //if there is no data, just write zeros to the output
        memset(config->tx.frame_buffer, 0, config->frame_buffer_size);
        state->tx.count++;
    }

    //these transfer members values may get modified by the driver
    state->tx.i2s_async.buf = config->tx.frame_buffer;
    state->tx.i2s_async.nbyte = config->frame_buffer_size;
    if( mcu_i2s_write(handle, &(state->tx.i2s_async)) < 0 ){
        state->tx.error = errno;
    }

    if( ret > 0 ){
        ffifo_data_transmitted(&(config->tx.ffifo), &(state->tx.ffifo));
    }

    //because mcu_i2s_write() reassigns the callback value -- returning 0 will null the callback
    return 1;
}

int i2s_event_data_ready(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;

    //write data to the receive buffer
    ffifo_write_buffer(&(config->rx.ffifo),
                       &(state->rx.ffifo),
                       config->rx.frame_buffer,
                       config->frame_buffer_size);

    state->rx.count++;

    state->rx.i2s_async.buf = config->rx.frame_buffer;
    state->rx.i2s_async.nbyte = config->frame_buffer_size;

    if( mcu_i2s_read(handle, &(state->rx.i2s_async)) < 0 ){
        state->rx.error = errno;
    }

    ffifo_data_received(&(config->rx.ffifo), &(state->rx.ffifo));

    return 1;
}

int i2s_ffifo_open(const devfs_handle_t * handle){
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;

    if( mcu_i2s_open(handle) <  0 ){ return -1; }

    if( ffifo_open_local(&config->tx.ffifo, &state->tx.ffifo) < 0 ){
        return -1;
    }

    return ffifo_open_local(&config->rx.ffifo, &state->rx.ffifo);
}

int i2s_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;
    i2s_ffifo_attr_t * attr = ctl;
    i2s_ffifo_info_t * info = ctl;
    mcu_action_t * action = ctl;

    switch(request){
    case I_I2S_SETATTR:
        if( mcu_i2s_ioctl(handle, request, ctl) < 0 ){
            return -1;
        }
        /* no break */
    case I_FFIFO_INIT:
        //the driver reads the I2S TX FIFO to write to the I2S
        if( config->tx.frame_buffer ){
            state->tx.i2s_async.loc = 0;
            state->tx.i2s_async.tid = task_get_current();
            state->tx.i2s_async.flags = O_RDWR;
            state->tx.i2s_async.buf = config->tx.frame_buffer;
            state->tx.i2s_async.nbyte = config->frame_buffer_size;
            state->tx.i2s_async.handler.context = (void*)handle;
            state->tx.i2s_async.handler.callback = i2s_event_write_complete;
            memset(config->tx.frame_buffer, 0, config->frame_buffer_size);

            state->tx.count = 0;
            state->tx.error = 0;

            state->tx.ffifo.read_async = 0;
            state->tx.ffifo.write_async = 0;
            ffifo_flush(&(state->tx.ffifo));

            //start writing data to the I2S -- zeros are written if there is no data in the fifo
            if( mcu_i2s_write(handle, &(state->tx.i2s_async)) < 0 ){
                return -1;
            }
        }


        if( config->rx.frame_buffer ){

            //the driver writes the I2S RX FIFO that is read from the I2S
            state->rx.i2s_async.loc = 0;
            state->rx.i2s_async.tid = task_get_current();
            state->rx.i2s_async.flags = O_RDWR;
            state->rx.i2s_async.buf = config->rx.frame_buffer;
            state->rx.i2s_async.nbyte = config->frame_buffer_size;
            state->rx.i2s_async.handler.context = (void*)handle;
            state->rx.i2s_async.handler.callback = i2s_event_data_ready;
            memset(config->rx.frame_buffer, 0, config->frame_buffer_size);

            state->rx.error = 0;
            state->rx.count = 0;


            state->rx.ffifo.read_async = 0;
            state->rx.ffifo.write_async = 0;
            ffifo_flush(&(state->rx.ffifo));


            //on the first call the FIFO is full of zeros and returns immediately
            if( mcu_i2s_read(handle, &(state->rx.i2s_async)) > 0 ){
                //start reading and running the callback
                if( mcu_i2s_read(handle, &(state->rx.i2s_async)) < 0 ){
                    return -1;
                }
            }
        }

        return 0;

    case I_FFIFO_FLUSH:
        ffifo_flush(&(state->tx.ffifo));
        ffifo_flush(&(state->rx.ffifo));
        return 0;

    case I_FFIFO_SETATTR:
        if( attr->tx.o_flags & FFIFO_FLAG_SET_WRITEBLOCK ){
            ffifo_set_writeblock(&(state->tx.ffifo), 1);
        } else {
            ffifo_set_writeblock(&(state->tx.ffifo), 0);
        }

        if( attr->rx.o_flags & FFIFO_FLAG_SET_WRITEBLOCK ){
            ffifo_set_writeblock(&(state->rx.ffifo), 1);
        } else {
            ffifo_set_writeblock(&(state->rx.ffifo), 0);
        }
        return 0;

    case I_FFIFO_GETINFO:
        ffifo_getinfo(&(info->tx.ffifo), &(config->tx.ffifo), &(state->tx.ffifo));
        ffifo_getinfo(&(info->rx.ffifo), &(config->rx.ffifo), &(state->rx.ffifo));
        info->tx.count = state->tx.count;
        info->rx.count = state->rx.count;
        info->tx.error = state->tx.error;
        info->rx.error = state->rx.error;
        return 0;

    case I_MCU_SETACTION:

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_DATA_READY) && (action->handler.callback == 0) ){
            //cancel the ffifo rx
            ffifo_cancel_rop(&(state->rx.ffifo));
        }

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE) && (action->handler.callback == 0) ){
            //cancel the ffifo rx
            ffifo_cancel_wop(&(state->tx.ffifo));
        }

        return 0;
    }

    return mcu_i2s_ioctl(handle, request, ctl);
}

int i2s_ffifo_read(const devfs_handle_t * handle, devfs_async_t * rop){
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;
    int ret;

    //disable interrupts to prevent elevated prio I2S from interrupting the read
    cortexm_disable_interrupts();
    ret = ffifo_read_local(&(config->rx.ffifo), &(state->rx.ffifo), rop, 0);
    cortexm_enable_interrupts();
    return ret;
}

int i2s_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async){
    int ret;
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;

    //I2S interrupt can't fire while writing the local buffer
    cortexm_disable_interrupts(0);
    ret = ffifo_write_local(&(config->tx.ffifo), &(state->tx.ffifo), async, 0);
    cortexm_enable_interrupts(0);
    return ret;
}

int i2s_ffifo_close(const devfs_handle_t * handle){
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;
    ffifo_close_local(&(config->tx.ffifo), &(state->tx.ffifo));
    ffifo_close_local(&(config->rx.ffifo), &(state->rx.ffifo));
    return mcu_i2s_close(handle);
}


