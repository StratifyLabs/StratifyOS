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
static void abort_connection(switchboard_state_t * state);
static void close_connection(switchboard_state_t * state);
static int destroy_connection(const switchboard_config_t * config, switchboard_state_t * state, u16 i);
static int clean_connections(const switchboard_config_t * config, switchboard_state_t * state);
static int open_terminal(const switchboard_state_terminal_t * state_terminal);
static int close_terminal(const switchboard_state_terminal_t * state_terminal);
static int get_terminal(const switchboard_config_t * config, const switchboard_state_terminal_t * state_terminal, switchboard_terminal_t * terminal);
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
static int check_for_stopped_or_destroyed(switchboard_state_t * state);

int switchboard_open(const devfs_handle_t * handle){
	//check to make sure everything is valid
	return 0;
}

int switchboard_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	const switchboard_config_t * config = handle->config;
	switchboard_state_t * state = handle->state;
	switchboard_attr_t * attr = ctl;
	switchboard_info_t * info = ctl;
	mcu_action_t * action = ctl;
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
			ret = 0;
			if( attr->id < config->connection_count ){
				if( o_flags & SWITCHBOARD_FLAG_CONNECT ){
					ret = create_connection(config, state, attr);
				} else if( o_flags & SWITCHBOARD_FLAG_DISCONNECT ){
					ret = destroy_connection(config, state, attr->id);
				} else if( o_flags & SWITCHBOARD_FLAG_CLEAN ){
					ret = clean_connections(config, state);
				}
			}
			return ret;

		case I_MCU_SETACTION:
		case I_SWITCHBOARD_SETACTION:
			if( action->channel < config->connection_count ){
				u16 id = action->channel;
				if( action->handler.callback == 0 ){
					mcu_execute_event_handler(&state[id].event_handler, MCU_EVENT_FLAG_CANCELED, 0);
				}

				state[id].event_handler = action->handler;
				return 0;
			} else {
				return SYSFS_SET_RETURN(EINVAL);
			}


	}
	return SYSFS_SET_RETURN(EINVAL);
}

int switchboard_read(const devfs_handle_t * handle, devfs_async_t * async){
	const switchboard_config_t * config = handle->config;
	switchboard_state_t * state = handle->state;
	if( (async->nbyte == sizeof(switchboard_connection_t)) && (async->loc % sizeof(switchboard_connection_t) == 0) ){
		u32 id = async->loc / sizeof(switchboard_connection_t);
		if( id < config->connection_count ){
			switchboard_status_t * status = async->buf;

			if( state[id].o_flags != 0 ){
				status->id = id;
				status->o_flags = state[id].o_flags;
				if( (state[id].o_flags & SWITCHBOARD_FLAG_IS_PERSISTENT) && (state[id].nbyte >= 0) ){
					status->nbyte = state[id].packet_size;
				} else {
					status->nbyte = state[id].nbyte;
				}

				if( get_terminal(config, &state[id].input, &status->input) < 0 ){
					return SYSFS_SET_RETURN(EIO);
				}

				if( get_terminal(config, &state[id].output, &status->output) < 0 ){
					return SYSFS_SET_RETURN(EIO);
				}

			} else {
				//connection is not used
				memset(status, 0, sizeof(switchboard_connection_t));
				status->id = id;
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
	if( terminal->priority > 0 ){
		memset(&action, 0, sizeof(action));
		action.channel = terminal->loc;
		action.prio = terminal->priority;
		action.o_events = o_events | MCU_EVENT_FLAG_SET_PRIORITY;
		//this will adjust the priority without affecting the callback (set to null)
		return device->driver.ioctl(&device->handle, I_MCU_SETACTION, &action);
	}
	return 0;
}

int create_connection(const switchboard_config_t * config, switchboard_state_t * state, const switchboard_attr_t * attr){
	u16 id = attr->id;
	int i;

	if( state[id].o_flags != 0 ){
		return SYSFS_SET_RETURN(EBUSY);
	}

	memset(state + id, 0, sizeof(switchboard_state_t));

	state[id].input.device = devfs_lookup_device(config->devfs_list, attr->input.name); //lookup input device from attr->input.name
	if( state[id].input.device == 0 ){
		return SYSFS_SET_RETURN(ENOENT);
	}

	state[id].output.device = devfs_lookup_device(config->devfs_list, attr->output.name); //lookup input device from attr->input.name
	if( state[id].output.device == 0 ){
		memset(state + id, 0, sizeof(switchboard_state_t));
		return SYSFS_SET_RETURN(ENOENT);
	}

	//check to see if the input or output is already an active connection
	for(i=0; i < config->connection_count; i++){
		if( i != id ){
			if( (state[id].input.device == state[i].input.device)  ){
				memset(state + id, 0, sizeof(switchboard_state_t));
				return SYSFS_SET_RETURN(EBUSY);
			}

			if( state[id].output.device == state[i].output.device ){
				memset(state + id, 0, sizeof(switchboard_state_t));
				return SYSFS_SET_RETURN(EBUSY);
			}
		}
	}

	if( attr->o_flags & SWITCHBOARD_FLAG_SET_TRANSACTION_LIMIT ){
		state[id].transaction_limit = attr->transaction_limit;
	} else {
		state[id].transaction_limit = config->transaction_limit;
	}

	state[id].buffer[0] = config->buffer + id*2*config->connection_buffer_size;
	state[id].buffer[1] = config->buffer + (id*2+1)*config->connection_buffer_size;

	state[id].nbyte = attr->nbyte; //total number of bytes to transfer OR packet size for persistent connections (must be less than buffer size)

	state[id].o_flags = SWITCHBOARD_FLAG_IS_CONNECTED;
	state[id].o_flags |= (attr->o_flags & (SWITCHBOARD_FLAG_IS_FILL_ZERO | SWITCHBOARD_FLAG_IS_INPUT_NON_BLOCKING | SWITCHBOARD_FLAG_IS_OUTPUT_NON_BLOCKING));

	if( attr->o_flags & SWITCHBOARD_FLAG_IS_PERSISTENT ){
		state[id].o_flags |= SWITCHBOARD_FLAG_IS_PERSISTENT;

		//for persistent connections, nbyte can't be larger than the buffer
		if( state[id].nbyte > config->connection_buffer_size ){
			state[id].nbyte = config->connection_buffer_size;
		}

		state[id].packet_size = state[id].nbyte;
		state[id].nbyte = 0;

	} else {

		//nbyte is total number of bytes
		state[id].packet_size = config->connection_buffer_size;
		if( state[id].packet_size > state[id].nbyte ){
			state[id].packet_size = state[id].nbyte;
		}
	}

	state[id].input.async.tid = task_get_current();
	state[id].input.async.flags = O_RDWR;
	if( attr->o_flags & SWITCHBOARD_FLAG_IS_INPUT_NON_BLOCKING ){
		state[id].input.async.flags |= O_NONBLOCK;
	}
	state[id].input.async.loc = attr->input.loc;
	state[id].input.async.handler.callback = handle_data_ready;
	state[id].input.async.handler.context = state + id;
	state[id].input.async.buf = state[id].buffer[0];
	state[id].input.async.nbyte = state[id].packet_size;

	memcpy(&state[id].output.async, &state[id].input.async, sizeof(devfs_async_t));

	//output is the same except the callback and the location (channel)
	state[id].output.async.handler.callback = handle_write_complete;
	state[id].output.async.loc = attr->output.loc;
	state[id].output.async.flags = O_RDWR;
	if( attr->o_flags & SWITCHBOARD_FLAG_IS_OUTPUT_NON_BLOCKING ){
		state[id].output.async.flags |= O_NONBLOCK;
	}

	if( open_terminal(&state->input) < 0 ){
		memset(state + id, 0, sizeof(switchboard_state_t));
		return SYSFS_SET_RETURN(EIO);
	}


	if( open_terminal(&state->output) < 0 ){
		close_terminal(&state->input);
		memset(state + id, 0, sizeof(switchboard_state_t));
		return SYSFS_SET_RETURN(EIO);
	}

	if( update_priority(state[id].input.device, &attr->input, MCU_EVENT_FLAG_DATA_READY) < 0 ){
		abort_connection(state + id);
		return SYSFS_SET_RETURN(EIO);
	}

	if( update_priority(state[id].output.device, &attr->output, MCU_EVENT_FLAG_WRITE_COMPLETE) < 0 ){
		abort_connection(state + id);
		return SYSFS_SET_RETURN(EIO);
	}

	//start reading into the primary buffer -- mark it as used
	int result;
	mcu_debug_log_info(MCU_DEBUG_DEVICE, "%d (%p) Starting %s -> %s", id, state + id, state[id].input.device->name, state[id].output.device->name);
	if( (result = read_then_write_until_async(state + id)) < 0 ){
		abort_connection(state + id);
		mcu_debug_log_error(MCU_DEBUG_DEVICE, "RW failed %d (0x%lX), %d", SYSFS_GET_RETURN(result), (u32)-1*SYSFS_GET_RETURN(result), SYSFS_GET_RETURN_ERRNO(result));
		return SYSFS_SET_RETURN(EIO);
	}

	return 0;
}

void abort_connection(switchboard_state_t * state){
	if( (state->o_flags & SWITCHBOARD_FLAG_IS_ERROR) == 0 ){
		close_terminal(&state->input);
		close_terminal(&state->output);
	}
	memset(state, 0, sizeof(switchboard_state_t));
}

int clean_connections(const switchboard_config_t * config, switchboard_state_t * state){
	u16 i;
	for(i=0; i < config->connection_count; i++){
		if( state[i].o_flags & SWITCHBOARD_FLAG_IS_ERROR ){
			destroy_connection(config, state, i);
		}
	}
	return 0;
}

int destroy_connection(const switchboard_config_t * config, switchboard_state_t * state, u16 id){

	if( id < config->connection_count ){

		if( state[id].o_flags & (SWITCHBOARD_FLAG_IS_ERROR|SWITCHBOARD_FLAG_IS_CANCELED) ){
			memset(state + id, 0, sizeof(switchboard_state_t));
		} else {
			//connection is still on going -- it will clear once it stops -- what happens if the connection never cleans up
			state[id].o_flags |= SWITCHBOARD_FLAG_IS_DESTROYED;
			state[id].nbyte = SYSFS_SET_RETURN(ENOEXEC); //force ongoing transactions to stop
		}
		return 0;
	}

	mcu_debug_log_info(MCU_DEBUG_DEVICE, "Invalid id:%d", id);
	return SYSFS_SET_RETURN(EINVAL);
}

void close_connection(switchboard_state_t * state){
	close_terminal(&state->input);
	close_terminal(&state->output);

	//connection is not connected anymore
	state->o_flags &= ~SWITCHBOARD_FLAG_IS_CONNECTED;

	//if connection has already been destroyed, reset the state -- otherwise state is reset when destroyed
	if( state->o_flags & SWITCHBOARD_FLAG_IS_DESTROYED ){
		memset(state, 0, sizeof(switchboard_state_t));
	}
}

int check_for_stopped_or_destroyed(switchboard_state_t * state){
	if( state->nbyte < 0 ){
		u32 o_events = MCU_EVENT_FLAG_STOP | MCU_EVENT_FLAG_CANCELED;
		mcu_debug_log_warning(MCU_DEBUG_DEVICE, "Stopping %s -> %s (%d, %d) 0x%lX", state->input.device->name, state->output.device->name, SYSFS_GET_RETURN(state->nbyte), SYSFS_GET_RETURN_ERRNO(state->nbyte), state->o_flags);

		if( state->o_flags & SWITCHBOARD_FLAG_IS_ERROR ){
			o_events |= MCU_EVENT_FLAG_ERROR;
		}

		close_connection(state);
		mcu_execute_event_handler(&state->event_handler, o_events, 0);
		return 1;
	}
	return 0;
}

int open_terminal(const switchboard_state_terminal_t * state_terminal){
	return state_terminal->device->driver.open(&state_terminal->device->handle);
}

int close_terminal(const switchboard_state_terminal_t * state_terminal){
	return state_terminal->device->driver.close(&state_terminal->device->handle);
}

int get_terminal(const switchboard_config_t * config,
					  const switchboard_state_terminal_t * state_terminal,
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

	if( (state->input.async.nbyte <= 0) || (state->nbyte < 0) ){
		//all reads complete or an error occurred
		return 0;
	}

	int buffer_is_free;
	if( state->input.async.buf == state->buffer[0] ){
		buffer_is_free = (state->bytes_in_buffer[0] == 0);
	} else {
		buffer_is_free = (state->bytes_in_buffer[1] == 0);
	}

	if( buffer_is_free == 0 ){
		//mcu_debug_root_printf("No buffers\n");
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
		return 0;
	}

	if( (state->output.async.nbyte < 0) || (state->nbyte < 0) ){
		//all writes are complete or an error occurred
		return 0;
	}

	if( state->output.async.buf == state->buffer[0] ){
		state->output.async.nbyte = state->bytes_in_buffer[0];
	} else {
		state->output.async.nbyte = state->bytes_in_buffer[1];
	}

	//there there are bytes in the buffer, then the device is ready to bw written
	return state->output.async.nbyte > 0;

}

void complete_read(switchboard_state_t * state, int bytes_read){
	update_bytes_transferred(state, &state->input);
	switch_input_buffer(state, bytes_read);
	if( state->input.async.nbyte > 0 ){
		state->input.async.nbyte = state->packet_size;
	}
}

void update_bytes_transferred(switchboard_state_t * state, switchboard_state_terminal_t * terminal){
	terminal->bytes_transferred += terminal->async.nbyte;
	if( state->nbyte > 0 ){ //state nbyte is set to 0 to persistnet connections
		//check to see how many bytes to transfer for non-persistent connections
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
	int errno_value;

	if( is_ready_to_write_device(state) ){ //is there a buffer with data that needs to be written?
		ret = state->output.device->driver.write(&state->output.device->handle, &state->output.async);
		if( ret == 0 ){
			//waiting for write
			state->o_flags |= SWITCHBOARD_FLAG_IS_WRITING_ASYNC;
		} else if( ret > 0 ){
			//buffer is free
			complete_write(state);
		} else {
			errno_value = SYSFS_GET_RETURN_ERRNO(ret);
			if( errno_value == EAGAIN ){
				//write device is set up in non-blocking mode -- just ignore the error and keep the connection going
				ret = 0;
			} else {
				state->nbyte = ret;
			}
		}
	}
	return ret;
}

int read_from_device(switchboard_state_t * state){
	//start writing the output device
	int ret = 0;
	int errno_value;

	if( is_ready_to_read_device(state) ){ //is there a buffer available
		ret = state->input.device->driver.read(&state->input.device->handle, &state->input.async);
		if( ret == 0 ){
			//the operation will happen asynchronously -- wait until it is done
			state->o_flags |= SWITCHBOARD_FLAG_IS_READING_ASYNC;
		} else if( ret > 0 ){
			//syncrhonous read completed
			complete_read(state, ret);
		} else {
			//there was an error reading the device
			errno_value = SYSFS_GET_RETURN_ERRNO(ret);
			int i;
			if( errno_value == EAGAIN ){
				//read device is set up in non-blocking mode
				if( state->o_flags & SWITCHBOARD_FLAG_IS_FILL_ZERO ){
					memset(state->input.async.buf, 0, state->input.async.nbyte);
				} else if( state->o_flags & SWITCHBOARD_FLAG_IS_FILL_LAST_8 ){
					int count = state->input.async.nbyte-1;
					u8 * ptr = state->input.async.buf;
					u8 value = ptr[count];
					for(i=0; i < count; i++){
						ptr[i] = value;
					}
				} else if( state->o_flags & SWITCHBOARD_FLAG_IS_FILL_LAST_16 ){
					int count = state->input.async.nbyte/sizeof(u16)-1;
					u16 * ptr = state->input.async.buf;
					u16 value = ptr[count];
					for(i=0; i < count; i++){
						ptr[i] = value;
					}
				} else if( state->o_flags & SWITCHBOARD_FLAG_IS_FILL_LAST_32 ){
					int count = state->input.async.nbyte/sizeof(u32)-1;
					u32 * ptr = state->input.async.buf;
					u32 value = ptr[count];
					for(i=0; i < count; i++){
						ptr[i] = value;
					}
				} else if( state->o_flags & SWITCHBOARD_FLAG_IS_FILL_LAST_64 ){
					int count = state->input.async.nbyte/sizeof(u64)-1;
					u64 * ptr = state->input.async.buf;
					u64 value = ptr[count];
					for(i=0; i < count; i++){
						ptr[i] = value;
					}
				} else {
					ret = 0;
				}


				if( state->o_flags | (SWITCHBOARD_FLAG_IS_FILL_ZERO |
											 SWITCHBOARD_FLAG_IS_FILL_LAST_8 |
											 SWITCHBOARD_FLAG_IS_FILL_LAST_16 |
											 SWITCHBOARD_FLAG_IS_FILL_LAST_32 |
											 SWITCHBOARD_FLAG_IS_FILL_LAST_64)){
					ret = state->input.async.nbyte;
					complete_read(state, ret);
				}

			} else {
				state->nbyte = ret;
			}
		}
	}
	return ret;
}

int read_then_write_until_async(switchboard_state_t * state){
	int ret;
	int transactions = 0;

	if( check_for_stopped_or_destroyed(state) ){
		return 0;
	}

	do {
		ret = read_from_device(state);
		if( ret == 0 ){ //read is either async or both buffers full
			ret = write_to_device(state);
		}
		transactions++;
	} while( ret > 0 && (transactions < state->transaction_limit) );

	if( transactions == state->transaction_limit ){
		state->nbyte = SYSFS_SET_RETURN(EDEADLK);
	} else {
		if( (state->o_flags & (SWITCHBOARD_FLAG_IS_WRITING_ASYNC|SWITCHBOARD_FLAG_IS_READING_ASYNC)) == 0 ){
			state->nbyte = SYSFS_SET_RETURN(ENODATA);
		}
	}

	check_for_stopped_or_destroyed(state);

	return ret;
}

int handle_data_ready(void * context, const mcu_event_t * event){
	switchboard_state_t * state = context;
	u32 o_events = event->o_events;

	//not waiting for ASYNC data to read anymore
	state->o_flags &= ~SWITCHBOARD_FLAG_IS_READING_ASYNC;

	if( (state->input.async.nbyte < 0) || (o_events & (MCU_EVENT_FLAG_CANCELED|MCU_EVENT_FLAG_ERROR)) ){
		//write error occurred -- abort connection

		if( o_events & MCU_EVENT_FLAG_ERROR ){
			state->o_flags |= SWITCHBOARD_FLAG_IS_ERROR;
		}

		if( o_events & MCU_EVENT_FLAG_CANCELED ){
			state->o_flags |= SWITCHBOARD_FLAG_IS_CANCELED;
		}

		if( state->input.async.nbyte < 0 ){
			state->nbyte = state->input.async.nbyte;
		} else {
			state->nbyte = SYSFS_SET_RETURN(EIO);
		}
	} else {
		complete_read(state, state->input.async.nbyte);
	}

	//this will try to start another read before writing in case there is a synchronous write delay
	read_then_write_until_async(state);

	return 0;
}

int handle_write_complete(void * context, const mcu_event_t * event){
	switchboard_state_t * state = context;
	u32 o_events = event->o_events;

	//not waiting for ASYNC data to write anymore
	state->o_flags &= ~SWITCHBOARD_FLAG_IS_WRITING_ASYNC;
	if( (state->output.async.nbyte < 0) || (o_events & (MCU_EVENT_FLAG_CANCELED|MCU_EVENT_FLAG_ERROR)) ){
		//write error occurred -- abort connection

		if( o_events & MCU_EVENT_FLAG_ERROR ){
			state->o_flags |= SWITCHBOARD_FLAG_IS_ERROR;
		}

		if( o_events & MCU_EVENT_FLAG_CANCELED ){
			state->o_flags |= SWITCHBOARD_FLAG_IS_CANCELED;
		}

		if( state->output.async.nbyte < 0 ){
			state->nbyte = state->output.async.nbyte;
		} else {
			mcu_debug_printf("sb events 0x%lX\n", o_events);
			state->nbyte = SYSFS_SET_RETURN(EIO);
		}

	} else {
		complete_write(state); //this frees the buffer that was just written

		//try to start another write operation in case there is a synchronous read delay
		write_to_device(state);
	}

	read_then_write_until_async(state);

	return 0;
}

