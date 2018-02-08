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
    if( (ret = ffifo_read_buffer(&(config->tx_ffifo),
            &(state->tx_ffifo),
            state->i2s_tx_frame_buffer,
            I2S_FFIFO_FRAME_SIZE)) <= 0 ){
        //if there is no data, just write zeros to the output
        memset(state->i2s_tx_frame_buffer, 0, I2S_FFIFO_FRAME_SIZE);
        state->tx_count++;
    }

    //these transfer members values may get modified by the driver
    state->i2s_write_async.buf = state->i2s_tx_frame_buffer;
    state->i2s_write_async.nbyte = I2S_FFIFO_FRAME_SIZE;
    if( mcu_i2s_write(handle, &(state->i2s_write_async)) < 0 ){
        state->tx_err = errno;
    }

    if( ret > 0 ){
        ffifo_data_transmitted(&(config->tx_ffifo), &(state->tx_ffifo));
    }

    //because mcu_i2s_write() reassigns the callback value -- returning 0 will null the callback
    return 1;
}

int i2s_event_data_ready(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    const i2s_ffifo_config_t * dcfg = handle->config;
    i2s_ffifo_state_t * state = handle->state;

    //write data to the receive buffer
    ffifo_write_buffer(&(dcfg->rx_ffifo),
            &(state->rx_ffifo),
            state->i2s_rx_frame_buffer,
            I2S_FFIFO_FRAME_SIZE);

    state->rx_count++;

    state->i2s_read_async.buf = state->i2s_rx_frame_buffer;
    state->i2s_read_async.nbyte = I2S_FFIFO_FRAME_SIZE;

    if( mcu_i2s_read(handle, &(state->i2s_read_async)) < 0 ){
        state->rx_err = errno;
    }

    ffifo_data_received(&(dcfg->rx_ffifo), &(state->rx_ffifo));

    return 1;
}

int i2s_ffifo_open(const devfs_handle_t * handle){
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;

    if( mcu_i2s_open(handle) <  0 ){ return -1; }

    if( ffifo_open_local(&config->tx_ffifo, &state->tx_ffifo) < 0 ){
        return -1;
    }

    return ffifo_open_local(&config->rx_ffifo, &state->rx_ffifo);
}

int i2s_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    const i2s_ffifo_config_t * dcfg = handle->config;
    i2s_ffifo_state_t * state = handle->state;
    i2s_ffifo_attr_t * attr = ctl;
    i2s_ffifo_info_t * info = ctl;
    mcu_action_t * action = ctl;

    switch(request){
    case I_FFIFO_INIT:
        //the driver reads the I2S TX FIFO to write to the I2S
        state->i2s_write_async.loc = 0;
        state->i2s_write_async.tid = task_get_current();
        state->i2s_write_async.flags = O_RDWR;
        state->i2s_write_async.buf = state->i2s_tx_frame_buffer;
        state->i2s_write_async.nbyte = I2S_FFIFO_FRAME_SIZE;
        state->i2s_write_async.handler.context = (void*)handle;
        state->i2s_write_async.handler.callback = i2s_event_write_complete;
        memset(state->i2s_tx_frame_buffer, 0, I2S_FFIFO_FRAME_SIZE);

        //the driver writes the I2S RX FIFO that is read from the I2S
        state->i2s_read_async.loc = 0;
        state->i2s_read_async.tid = task_get_current();
        state->i2s_read_async.flags = O_RDWR;
        state->i2s_read_async.buf = state->i2s_rx_frame_buffer;
        state->i2s_read_async.nbyte = I2S_FFIFO_FRAME_SIZE;
        state->i2s_read_async.handler.context = (void*)handle;
        state->i2s_read_async.handler.callback = i2s_event_data_ready;
        memset(state->i2s_rx_frame_buffer, 0, I2S_FFIFO_FRAME_SIZE);

        state->tx_count = 0;
        state->rx_count = 0;
        state->tx_err = 0;
        state->rx_err = 0;

        state->tx_ffifo.read_async = NULL;
        state->tx_ffifo.write_async = NULL;
        state->rx_ffifo.read_async = NULL;
        state->rx_ffifo.write_async = NULL;
        ffifo_flush(&(state->tx_ffifo));
        ffifo_flush(&(state->rx_ffifo));

#if 0
        //start writing data to the I2S -- zeros are written if there is no data in the fifo
        if( mcu_i2s_write(handle, &(state->i2s_write_op)) < 0 ){
            return -1;
        }

        //on the first call the FIFO is full of zeros
        if( mcu_i2s_read(handle, &(state->i2s_read_op)) > 0 ){
            //start reading and running the callback
            if( mcu_i2s_read(handle, &(state->i2s_read_op)) < 0 ){
                return -1;
            }
        }
#endif
        return 0;

    case I_FFIFO_FLUSH:
        ffifo_flush(&(state->tx_ffifo));
        ffifo_flush(&(state->rx_ffifo));
        return 0;

    case I_FFIFO_SETATTR:
        if( attr->tx.o_flags & FFIFO_FLAG_SET_WRITEBLOCK ){
            ffifo_set_writeblock(&(state->tx_ffifo), 1);
        } else {
            ffifo_set_writeblock(&(state->tx_ffifo), 0);
        }

        if( attr->rx.o_flags & FFIFO_FLAG_SET_WRITEBLOCK ){
            ffifo_set_writeblock(&(state->rx_ffifo), 1);
        } else {
            ffifo_set_writeblock(&(state->rx_ffifo), 0);
        }
        return 0;

    case I_FFIFO_GETINFO:
        ffifo_getinfo(&(info->tx), &(dcfg->tx_ffifo), &(state->tx_ffifo));
        ffifo_getinfo(&(info->rx), &(dcfg->rx_ffifo), &(state->rx_ffifo));
        info->tx_count = state->tx_count;
        info->rx_count = state->rx_count;
        info->tx_err = state->tx_err;
        info->rx_err = state->rx_err;
        return 0;

    case I_MCU_SETACTION:

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_DATA_READY) && (action->handler.callback == 0) ){
            //cancel the ffifo rx
            ffifo_cancel_rop(&(state->rx_ffifo));
        }

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE) && (action->handler.callback == 0) ){
            //cancel the ffifo rx
            ffifo_cancel_wop(&(state->tx_ffifo));
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
    cortexm_disable_interrupts(0);
    ret = ffifo_read_local(&(config->rx_ffifo), &(state->rx_ffifo), rop, 0);
    cortexm_enable_interrupts(0);
    return ret;
}

int i2s_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async){
    int ret;
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;

    //I2S interrupt can't fire while writing the local buffer
    cortexm_disable_interrupts(0);
    ret = ffifo_write_local(&(config->tx_ffifo), &(state->tx_ffifo), async, 0);
    cortexm_enable_interrupts(0);
    return ret;
}

int i2s_ffifo_close(const devfs_handle_t * handle){
    const i2s_ffifo_config_t * config = handle->config;
    i2s_ffifo_state_t * state = handle->state;
    ffifo_close_local(&(config->tx_ffifo), &(state->tx_ffifo));
    ffifo_close_local(&(config->rx_ffifo), &(state->rx_ffifo));
    return mcu_i2s_close(handle);
}


