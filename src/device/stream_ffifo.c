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
	ffifo_state_t * ffifo_state = &state->tx.ffifo;

	if(state->tx.async.nbyte < 0){
		state->tx.error = state->tx.async.nbyte;
		mcu_debug_log_error(MCU_DEBUG_DEVICE, "error: %s():%d", __FUNCTION__, __LINE__);
		return 0;
	}

	u32 frame_count = config->tx.frame_count;
	if( event->o_events & (MCU_EVENT_FLAG_LOW | MCU_EVENT_FLAG_HIGH) ){
		frame_count >>= 1; //frame count/2
	}

	//check for underflow
	if( ffifo_state->atomic_position.access.tail != config->tx.frame_count ){ //buffer should be full when this event fires -- if not fill it with zeros
		char frame[config->tx.frame_size];
		ffifo_state->o_flags |= FIFO_FLAG_IS_OVERFLOW;
		memset(frame, 0, config->tx.frame_size);
		while( ffifo_state->atomic_position.access.tail != config->tx.frame_count ){
			//if buffer is not full -- make it full of zeros -- what happens if application is writing while this write -- interrupt priority?
			ffifo_write_buffer(&config->tx, ffifo_state, frame, config->tx.frame_size);
		}
	}

	for(u32 frames = 0; frames < frame_count; frames++){
		state->tx.access_count++;
		//increment the tail for the frame that was written
		if( ffifo_state->atomic_position.access.tail == config->tx.frame_count ){
			ffifo_state->atomic_position.access.tail = ffifo_state->atomic_position.access.head;
		}
		ffifo_inc_tail(ffifo_state, config->tx.frame_count);

#if 0 //this should never print anything -- can be used for debugging
		if( event->o_events & MCU_EVENT_FLAG_LOW ){
			if( ffifo_state->atomic_position.access.head != 0 ){
				mcu_debug_printf("head not zero 0x%lX\n", ffifo_state->o_flags);
			}
		} else {
			if( ffifo_state->atomic_position.access.head == 0 ){
				mcu_debug_printf("head is zero\n");
			}
		}
#endif

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
	if( ffifo_state->atomic_position.access.tail != ffifo_state->atomic_position.access.head ){ //buffer should be empty
		if( ffifo_state->o_flags & FIFO_FLAG_IS_READ_BUSY ){
			ffifo_state->o_flags |= FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY;
		}
		ffifo_state->o_flags |= FIFO_FLAG_IS_OVERFLOW;
		ffifo_state->atomic_position.access.tail = ffifo_state->atomic_position.access.head; //forces the buffer to be empty
	}

	//increment the head for the frame received
	u32 frame_count = config->rx.frame_count;
	if( o_events & (MCU_EVENT_FLAG_LOW | MCU_EVENT_FLAG_HIGH) ){
		frame_count >>= 1;
	}

	for(u32 frames = 0; frames < frame_count; frames++){
		ffifo_inc_head(ffifo_state, config->rx.frame_count);
		state->rx.access_count++;
		ffifo_data_received(&config->rx, ffifo_state);
	}


	return 1;
}

int stream_ffifo_open(const devfs_handle_t * handle){
	const stream_ffifo_config_t * config = handle->config;
	stream_ffifo_state_t * state = handle->state;
	int ret;

	if( config == 0 ){ return SYSFS_SET_RETURN(ENOSYS); }
	if( state == 0 ){ return SYSFS_SET_RETURN(ENOSYS); }

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
	int result;

	switch(request){

		case I_STREAM_FFIFO_GETVERSION: return STREAM_FFIFO_VERSION;
		case I_STREAM_FFIFO_SETATTR:

			if( attr->o_flags & STREAM_FFIFO_FLAG_START ){

				mcu_debug_log_info(MCU_DEBUG_DEVICE, "Start Stream");

				if( config->tx.buffer ){
					//The application writes data to the FIFO that is then read by the hardware and written to a device
					state->tx.async.loc = config->tx_loc;
					state->tx.async.tid = task_get_current();
					state->tx.async.flags = O_RDWR;
					state->tx.async.buf = config->tx.buffer;
					state->tx.async.nbyte = config->tx.frame_size * config->tx.frame_count;
					state->tx.async.handler.context = (void*)handle;
					state->tx.async.handler.callback = event_write_complete;

					//write block is true but the function doesn't block -- returns EGAIN if it is written while full
					ffifo_set_writeblock(&(state->tx.ffifo), 1);

					state->tx.access_count = 0;
					state->tx.error = 0;

					state->tx.ffifo.transfer_handler.read = 0;
					state->tx.ffifo.transfer_handler.write = 0;
					ffifo_flush(&(state->tx.ffifo));

					//ffifo is empty so the head must be incremented on start to be in the middle
					//There needs to be a for loop here to inc the head to the halfway point in case tx.frame_count != 2
					u32 i;
					for(i=0; i < config->tx.frame_count/2; i++){
						ffifo_inc_head(&state->tx.ffifo, config->tx.frame_count);
					}
					ffifo_set_writeblock(&state->tx.ffifo, 1);

					//start writing data to the driver -- zeros comprise the first frame
					memset(state->tx.async.buf, 0, state->tx.async.nbyte);

					result = config->device->driver.write(&config->device->handle, &(state->tx.async));
					if( result < 0 ){ return result; }
				}


				if( config->rx.buffer ){
					//the application reads the RX buffer the is written by the device and data that is read from hardware
					state->rx.async.loc = config->rx_loc;
					state->rx.async.tid = task_get_current();
					state->rx.async.flags = O_RDWR;
					state->rx.async.buf = config->rx.buffer;
					state->rx.async.nbyte = config->rx.frame_size * config->rx.frame_count;
					state->rx.async.handler.context = (void*)handle;
					state->rx.async.handler.callback = event_data_ready;

					state->rx.error = 0;
					state->rx.access_count = 0;

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
			info->tx.access_count = state->tx.access_count;
			info->rx.access_count = state->rx.access_count;
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
			/* no break */ //execute the driver SETACTION as well
	}

	return config->device->driver.ioctl(&config->device->handle, request, ctl);
}

int stream_ffifo_read(const devfs_handle_t * handle, devfs_async_t * async){
	const stream_ffifo_config_t * config = handle->config;
	stream_ffifo_state_t * state = handle->state;

	if( config->rx.buffer == 0 ){ return SYSFS_SET_RETURN(ENOSYS); }

	//this will never need to execute a callback because the FIFO is written by hardware
	return ffifo_read_local(&(config->rx), &(state->rx.ffifo), async, 0);
}

int stream_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async){
	const stream_ffifo_config_t * config = handle->config;
	stream_ffifo_state_t * state = handle->state;

	if( config->tx.buffer == 0 ){ return SYSFS_SET_RETURN(ENOSYS); }

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


