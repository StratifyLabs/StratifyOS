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
#include "device/switchboard.h"
#include "mcu/debug.h"


static int create_connection(const switchboard_config_t * config, switchboard_state_t * state, const switchboard_attr_t * attr);
static int destroy_connection(const switchboard_config_t * config, switchboard_state_t * state, const switchboard_attr_t * attr);
static int get_terminal(const switchboard_config_t * config, switchboard_state_terminal_t * state_terminal, switchboard_terminal_t * terminal);
static int update_priority(const devfs_device_t * device, const switchboard_terminal_t * terminal, u32 o_events);
static int handle_data_ready(void * context, const mcu_event_t * event);
static int handle_write_complete(void * context, const mcu_event_t * event);
static int read_then_write_until_async(switchboard_state_t * state);
static void complete_read(switchboard_state_t * state, int bytes_read);
static void complete_write(switchboard_state_t * state);
static void update_bytes_transferred(switchboard_state_t * state, switchboard_state_terminal_t * terminal);
static void switch_input_buffer(switchboard_state_t * state, int bytes_read);
static void switch_output_buffer(switchboard_state_t * state);
static int write_to_device(switchboard_state_t * state);

int switchboard_open(const devfs_handle_t * handle){

    //check to make sure everything is valid

    return 0;
}

int switchboard_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    const switchboard_config_t * config = handle->config;
    switchboard_state_t * state = handle->state;
    switchboard_attr_t * attr = ctl;
    switchboard_info_t * info = ctl;
    int ret;
    u32 o_flags;

    switch(request){
    case I_SWITCHBOARD_GETVERSION:
        return SWITCHBOARD_VERSION;

    case I_SWITCHBOARD_GETINFO:
        info->o_flags = SWITCHBOARD_FLAG_CONNECT |
                SWITCHBOARD_FLAG_DISCONNECT |
                SWITCHBOARD_FLAG_IS_PERSISTENT;
        info->connection_count = config->connection_count;
        info->connection_buffer_size = config->connection_buffer_size;
        info->transaction_limit = config->transaction_limit;
        return 0;

    case I_SWITCHBOARD_SETATTR:
        o_flags = attr->o_flags;

        if( o_flags & SWITCHBOARD_FLAG_CONNECT ){
            //is connection idx available?

            if( attr->idx < config->connection_count ){
                return create_connection(config, state, attr);
            } else {
                return SYSFS_SET_RETURN(EINVAL);
            }

        } else if( o_flags & SWITCHBOARD_FLAG_DISCONNECT ){
            ret = destroy_connection(config, state, attr);
            if( ret < 0 ){
                return ret;
            }
        }
        break;

    default:
        return 0;
    }
    return 0;
}

int switchboard_read(const devfs_handle_t * handle, devfs_async_t * async){
    const switchboard_config_t * config = handle->config;
    switchboard_state_t * state = handle->state;
    if( (async->nbyte == sizeof(switchboard_connection_t)) && (async->loc % sizeof(switchboard_connection_t) == 0) ){
        u32 idx = async->loc / sizeof(switchboard_connection_t);
        if( idx < config->connection_count ){
            switchboard_status_t * status = async->buf;

            if( state->o_flags != 0 ){
                status->idx = idx;
                state->o_flags = state[idx].o_flags;
                if( state[idx].o_flags & SWITCHBOARD_FLAG_IS_PERSISTENT ){
                    state->nbyte = state[idx].input.async.nbyte;
                } else {
                    status->nbyte = state[idx].nbyte;
                }

                if( get_terminal(config, &state[idx].input, &status->input) < 0 ){
                    return SYSFS_SET_RETURN(EIO);
                }
                if( get_terminal(config, &state[idx].output, &status->output) < 0 ){
                    return SYSFS_SET_RETURN(EIO);
                }
            } else {
                //connection is not used
                memset(status, 0, sizeof(switchboard_connection_t));
                status->idx = idx;
            }
            return sizeof(switchboard_connection_t);

        } else {
            return SYSFS_RETURN_EOF;
        }
    }
    return SYSFS_SET_RETURN(EINVAL);
}

int switchboard_write(const devfs_handle_t * handle, devfs_async_t * async){
    return SYSFS_SET_RETURN(ENOTSUP);
}

int switchboard_close(const devfs_handle_t * handle){
    return 0;
}



int update_priority(const devfs_device_t * device, const switchboard_terminal_t * terminal, u32 o_events){
    mcu_action_t action;
    memset(&action, 0, sizeof(action));
    action.channel = terminal->loc;
    action.prio = terminal->priority;
    action.o_events = o_events;
    //this will adjust the priority without affecting the callback (set to null)
    return device->driver.ioctl(&device->handle, I_MCU_SETACTION, &action);
}

int create_connection(const switchboard_config_t * config, switchboard_state_t * state, const switchboard_attr_t * attr){
    int idx = attr->idx;

    if( state[idx].o_flags != 0 ){
        return SYSFS_SET_RETURN(EBUSY);
    }

    memset(state + idx, 0, sizeof(switchboard_state_t));

    state[idx].input.device = devfs_lookup_device(config->devfs_list, attr->input.name); //lookup input device from attr->input.name
    if( state[idx].input.device == 0 ){
        return SYSFS_SET_RETURN(ENOENT);
    }

    state[idx].output.device = devfs_lookup_device(config->devfs_list, attr->output.name); //lookup input device from attr->input.name
    if( state[idx].output.device == 0 ){
        return SYSFS_SET_RETURN(ENOENT);
    }

    if( update_priority(state[idx].input.device, &attr->input, MCU_EVENT_FLAG_DATA_READY) < 0 ){
        return SYSFS_SET_RETURN(EIO);
    }

    if( update_priority(state[idx].output.device, &attr->output, MCU_EVENT_FLAG_WRITE_COMPLETE) < 0 ){
        return SYSFS_SET_RETURN(EIO);
    }

    if( attr->o_flags & SWITCHBOARD_FLAG_SET_TRANSACTION_LIMIT ){
        state[idx].transaction_limit = attr->transaction_limit;
    } else {
        state[idx].transaction_limit = config->transaction_limit;
    }

    state[idx].buffer[0] = config->buffer + idx*2*config->connection_buffer_size;
    state[idx].buffer[1] = config->buffer + (idx*2+1)*config->connection_buffer_size;

    state[idx].nbyte = attr->nbyte; //total number of bytes to transfer OR packet size for persistent connections (must be less than buffer size)

    state[idx].o_flags = SWITCHBOARD_FLAG_IS_CONNECTED;
    if( attr->o_flags & SWITCHBOARD_FLAG_IS_PERSISTENT ){
        state[idx].o_flags |= SWITCHBOARD_FLAG_IS_PERSISTENT;

        //for persistent connections, nbyte can't be larger than the buffer
        if( state[idx].nbyte > config->connection_buffer_size ){
            state[idx].nbyte = config->connection_buffer_size;
        }

        state[idx].packet_size = state[idx].nbyte;
        if( state[idx].packet_size > config->connection_buffer_size ){
            state[idx].packet_size = config->connection_buffer_size;
        }

        state[idx].nbyte = 0;

    } else {

        //nbyte is total number of bytes
        state[idx].packet_size = config->connection_buffer_size;
        if( state[idx].packet_size > state[idx].nbyte ){
            state[idx].packet_size = state[idx].nbyte;
        }
    }

    state[idx].input.async.tid = task_get_current();
    state[idx].input.async.flags = O_RDWR;
    state[idx].input.async.loc = attr->input.loc;
    state[idx].input.async.handler.callback = handle_data_ready;
    state[idx].input.async.handler.context = state + idx;
    state[idx].input.async.buf = state[idx].buffer[0];
    state[idx].input.async.nbyte = state[idx].packet_size;

    memcpy(&state[idx].output.async, &state[idx].input.async, sizeof(devfs_async_t));

    //output is the same except the callback and the location (channel)
    state[idx].output.async.handler.callback = handle_write_complete;
    state[idx].output.async.loc = attr->output.loc;


    //start reading into the primary buffer -- mark it as used
    if( read_then_write_until_async(state + idx) < 0 ){
        return SYSFS_SET_RETURN(EIO);
    }

    return 0;
}

int destroy_connection(const switchboard_config_t * config, switchboard_state_t * state, const switchboard_attr_t * attr){
    int idx = attr->idx;
    state[idx].o_flags = 0;
    state[idx].nbyte = -1; //force ongoing transactions to stop
    return 0;
}

int get_terminal(const switchboard_config_t * config,
                 switchboard_state_terminal_t * state_terminal,
                 switchboard_terminal_t * terminal){
    terminal->loc = state_terminal->async.loc;
    terminal->bytes_transferred = state_terminal->bytes_transferred;
    return devfs_lookup_name(config->devfs_list, state_terminal->device, terminal->name);
}

//switch happens after data is read
int is_ready_to_read_device(switchboard_state_t * state){

    if( state->o_flags & SWITCHBOARD_FLAG_IS_READING_ASYNC ){
        //a read is already in progress
        return 0;
    }

    if( state->input.async.nbyte < 0 ){
        //all reads complete or an error occurred
        return 0;
    }

    int buffer_is_free;
    if( state->input.async.buf == state->buffer[0] ){
        buffer_is_free = (state->bytes_in_buffer[0] == 0);
    } else {
        buffer_is_free = (state->bytes_in_buffer[1] == 0);
    }
    return buffer_is_free;
}

//switch happens after data is written -- so previous buffer will be unused
void switch_input_buffer(switchboard_state_t * state, int bytes_read){
    if( state->input.async.buf == state->buffer[0] ){
        //the read completed on this buffer -- set the number of bytes read
        state->bytes_in_buffer[0] = bytes_read;

        state->input.async.buf = state->buffer[1];
    } else {
        state->bytes_in_buffer[1] = bytes_read;
        state->input.async.buf = state->buffer[0];
    }
}

//switch happens after data is written -- so previous buffer will be unused
void switch_output_buffer(switchboard_state_t * state){
    if( state->output.async.buf == state->buffer[0] ){
        state->bytes_in_buffer[0] = 0; //bytes were written
        state->output.async.buf = state->buffer[1];
    } else {
        state->bytes_in_buffer[1] = 0; //bytes were written
        state->output.async.buf = state->buffer[0];
    }
}

int is_ready_to_write_device(switchboard_state_t * state){

    if( state->o_flags & SWITCHBOARD_FLAG_IS_WRITING_ASYNC ){
        //a write is already in progress
        mcu_debug_root_printf("write busy async\n");
        return 0;
    }

    if( state->output.async.nbyte < 0 ){
        //all writes are complete or an error occurred
        mcu_debug_root_printf("write error async\n");
        return 0;
    }

    if( state->output.async.buf == state->buffer[0] ){
        state->output.async.nbyte = state->bytes_in_buffer[0];
    } else {
        state->output.async.nbyte = state->bytes_in_buffer[1];
    }

    mcu_debug_root_printf("write bytes: %d\n", state->output.async.nbyte);

    //there there are bytes in the buffer, then the device is ready to bw written
    return state->output.async.nbyte > 0;

}

void complete_read(switchboard_state_t * state, int bytes_read){
    update_bytes_transferred(state, &state->input);
    switch_input_buffer(state, bytes_read);
}

void update_bytes_transferred(switchboard_state_t * state, switchboard_state_terminal_t * terminal){
    terminal->bytes_transferred += terminal->async.nbyte;
    if( state->nbyte > 0 ){
        //check to see how many bytes to read
        if( state->nbyte - terminal->bytes_transferred > terminal->async.nbyte ){
            terminal->async.nbyte = state->nbyte - terminal->bytes_transferred;
        }
    } else if( state->nbyte < 0 ){
        terminal->async.nbyte = -1;
    }
}

void complete_write(switchboard_state_t * state){
    update_bytes_transferred(state, &state->output);

    //switches and marks the buffer as unused (ready for read device to write to buffer)
    switch_output_buffer(state);
}


int write_to_device(switchboard_state_t * state){
    //start writing the output device
    int ret = 0;

    if( is_ready_to_write_device(state) ){ //is there a buffer with data that needs to be written?
        mcu_debug_root_printf("write:%d\n", state->output.async.nbyte);
        ret = state->output.device->driver.write(&state->output.device->handle, &state->output.async);
        if( ret == 0 ){
            //waiting for write
            state->o_flags |= SWITCHBOARD_FLAG_IS_WRITING_ASYNC;
        } else if( ret > 0 ){
            //buffer is free
            complete_write(state);
        }
    }
    return ret;
}

int read_from_device(switchboard_state_t * state){
    //start writing the output device
    int ret = 0;

    if( is_ready_to_read_device(state) ){ //is there a buffer available
        mcu_debug_root_printf("read %d\n", state->input.async.nbyte);
        ret = state->input.device->driver.read(&state->input.device->handle, &state->input.async);
        if( ret == 0 ){
            //waiting for write
            state->o_flags |= SWITCHBOARD_FLAG_IS_READING_ASYNC;
            mcu_debug_root_printf("read async\n");
        } else if( ret > 0 ){
            //syncrhonous read completed
            complete_read(state, ret);
        }
    }
    return ret;
}

int read_then_write_until_async(switchboard_state_t * state){
    int ret;
    int transactions = 0;

    //an error has occurred -- abort the connection
    if( state->nbyte < 0 ){
        return 0;
    }

    do {
        ret = read_from_device(state);
        if( ret == 0 ){ //read is either async or both buffers full
            ret = write_to_device(state);
            //if write is sync (ret > 0) and a buffer will be available for reading
            //if write is async (ret == 0) and connection needs to wait until data is done writing
            //if error (ret < 0 ) -- writing is stopped -- still might have SWITCHBOARD_FLAG_IS_READING_ASYNC
        }
        transactions++;
    } while( ret > 0 && (transactions < state->transaction_limit) );

    if( ret < 0 ){
        state->nbyte = ret;
    }

    return ret;
}

int handle_data_ready(void * context, const mcu_event_t * event){
    switchboard_state_t * state = context;

    //not waiting for ASYNC data to read anymore
    state->o_flags &= ~SWITCHBOARD_FLAG_IS_READING_ASYNC;

    mcu_debug_root_printf("data ready %d\n", state->input.async.nbyte);

    if( state->input.async.nbyte < 0 ){
        //error -- no more action
        state->nbyte = state->input.async.nbyte;
    } else {
        complete_read(state, state->input.async.nbyte);

        //restore async to packet size if less than the packet size was read
        state->input.async.nbyte = state->packet_size;

        //this will try to start another read before writing in case there is a synchronous write delay
        read_then_write_until_async(state);
    }

    return 0;
}

int handle_write_complete(void * context, const mcu_event_t * event){
    switchboard_state_t * state = context;

    //not waiting for ASYNC data to write anymore
    state->o_flags &= ~SWITCHBOARD_FLAG_IS_WRITING_ASYNC;
    if( state->output.async.nbyte < 0 ){
        //write error occurred -- abort connection
        state->nbyte = state->output.async.nbyte;
    } else {
        complete_write(state); //this frees the buffer that was just written

        //try to start another write operation in case there is a synchronous read delay
        write_to_device(state);

        read_then_write_until_async(state);
    }

    return 0;
}

