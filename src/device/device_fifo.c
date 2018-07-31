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
#include "device/device_fifo.h"

static int event_write_complete(void * context, const mcu_event_t * event);
static int event_data_ready(void * context, const mcu_event_t * event);

int event_write_complete(void * context, const mcu_event_t * event){
    //const devfs_handle_t * handle = context;
    //const device_fifo_config_t * config = handle->config;
    //device_fifo_state_t * state = handle->state;


    //read more data from the FIFO and write to the device

    //write from tail to end or from tail to head


    return 1;
}

int event_data_ready(void * context, const mcu_event_t * event){
    // devfs_handle_t * handle = context;
    //const device_fifo_config_t * config = handle->config;
    //device_fifo_state_t * state = handle->state;
    //fifo_state_t * fifo_state = &state->rx.fifo;


    //adjust the head for the data that was read

    //adjust the async->buf and nbyte to avoid overflow



    return 0;
}

int device_fifo_open(const devfs_handle_t * handle){
    const device_fifo_config_t * config = handle->config;
    device_fifo_state_t * state = handle->state;
    int result;

    result = config->device->driver.open(&config->device->handle);
    if( result <  0 ){ return result; }

    if( config->tx.buffer ){
        result = fifo_open_local(&config->tx, &state->tx.fifo);
        if( result < 0 ){ return result; }
    }

    if( config->rx.buffer ){
        result = fifo_open_local(&config->rx, &state->rx.fifo);
        if( result < 0 ){ return result; }
    }

    return SYSFS_RETURN_SUCCESS;
}

int device_fifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    const device_fifo_config_t * config = handle->config;
    device_fifo_state_t * state = handle->state;
    device_fifo_info_t * info = ctl;
    const device_fifo_attr_t * attr = ctl;
    mcu_action_t * action = ctl;

    switch(request){

    case I_DEVICE_FIFO_GETVERSION: return DEVICE_FIFO_VERSION;
    case I_DEVICE_FIFO_SETATTR:

        if( attr->o_flags & DEVICE_FIFO_FLAG_START ){

            mcu_debug_log_info(MCU_DEBUG_DEVICE, "Start Stream");

            if( config->rx.buffer ){

                //the driver writes the I2S RX FIFO that is read from the I2S
                state->rx.async.loc = 0;
                state->rx.async.tid = task_get_current();
                state->rx.async.flags = O_RDWR;
                state->rx.async.buf = config->rx.buffer;
                state->rx.async.nbyte = config->rx.size;
                state->rx.async.handler.context = (void*)handle;
                state->rx.async.handler.callback = event_data_ready;

                state->rx.error = 0;
                state->rx.count = 0;

                state->rx.fifo.transfer_handler.read = 0;
                state->rx.fifo.transfer_handler.write = 0;
                fifo_flush(&(state->rx.fifo));

                //start reading the underlying device and writing data directly the fifo
                if( config->device->driver.read(&config->device->handle, &(state->rx.async)) < 0 ){
                    return SYSFS_SET_RETURN(EIO);
                }
            }

            if( config->tx.buffer ){
                state->tx.async.loc = 0;
                state->tx.async.tid = task_get_current();
                state->tx.async.flags = O_RDWR;
                state->tx.async.buf = config->tx.buffer;
                state->tx.async.nbyte = config->tx.size;
                state->tx.async.handler.context = (void*)handle;
                state->tx.async.handler.callback = event_write_complete;

                fifo_set_writeblock(&(state->tx.fifo), 1);

                state->tx.count = 0;
                state->tx.error = 0;

                state->tx.fifo.transfer_handler.read = 0;
                state->tx.fifo.transfer_handler.write = 0;
                fifo_flush(&(state->tx.fifo));

                //Start a transfer with all zeros in the buffer

            }

        } else if( attr->o_flags & DEVICE_FIFO_FLAG_STOP ){

            //stop reading and writing data -- cancel ongoing transfers

        }

        if( attr->o_flags & DEVICE_FIFO_FLAG_FLUSH ){
            fifo_flush(&(state->tx.fifo));
            fifo_flush(&(state->rx.fifo));

            state->rx.async.buf = config->rx.buffer;
            state->rx.async.nbyte = config->rx.size;

            state->tx.async.buf = config->tx.buffer;
            state->tx.async.nbyte = config->tx.size;
        }


        return 0;

    case I_DEVICE_FIFO_GETINFO:
        fifo_getinfo(&(info->tx.fifo), &(config->tx), &(state->tx.fifo));
        fifo_getinfo(&(info->rx.fifo), &(config->rx), &(state->rx.fifo));
        info->tx.count = state->tx.count;
        info->rx.count = state->rx.count;
        info->tx.error = state->tx.error;
        info->rx.error = state->rx.error;
        return 0;

    case I_DEVICE_FIFO_SETACTION:
    case I_MCU_SETACTION:

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_DATA_READY) && (action->handler.callback == 0) ){
            //cancel the fifo rx
            fifo_cancel_async_read(&(state->rx.fifo));
        }

        //this needs to handle cancelling wop/rop of tx and rx fifos
        if( (action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE) && (action->handler.callback == 0) ){
            //cancel the fifo rx
            fifo_cancel_async_write(&(state->tx.fifo));
        }
        /* no break */
    }

    return config->device->driver.ioctl(&config->device->handle, request, ctl);
}

int device_fifo_read(const devfs_handle_t * handle, devfs_async_t * async){
    const device_fifo_config_t * config = handle->config;
    device_fifo_state_t * state = handle->state;
    int result = 0;

    result = fifo_read_local(&(config->rx), &(state->rx.fifo), async, 1);

    return result;

}

int device_fifo_write(const devfs_handle_t * handle, devfs_async_t * async){
    const device_fifo_config_t * config = handle->config;
    device_fifo_state_t * state = handle->state;
    int result;

    result = fifo_write_local(&(config->tx), &(state->tx.fifo), async, 1);

    //is device busy -- start writing data from the FIFO to the device

    return result;
}

int device_fifo_close(const devfs_handle_t * handle){
    const device_fifo_config_t * config = handle->config;
    device_fifo_state_t * state = handle->state;
    fifo_close_local(&(config->tx), &(state->tx.fifo));
    fifo_close_local(&(config->rx), &(state->rx.fifo));
    return config->device->driver.close(&config->device->handle);
}


