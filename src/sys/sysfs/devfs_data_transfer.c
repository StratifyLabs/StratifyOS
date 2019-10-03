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


#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include "mcu/debug.h"

#include "mcu/mcu.h"
#include "../scheduler/scheduler_local.h"

#include "sos/fs/sysfs.h"
#include "devfs_local.h"

#define ARGS_READ_WRITE 0
#define ARGS_READ_READ 1
#define ARGS_READ_DONE 2

typedef struct {
	const void * device;
	devfs_async_t async;
	volatile int is_read;
	int result;
} svcall_device_data_transfer_t;


static void svcall_check_op_complete(void * args);
static void root_check_op_complete(void * args);
static void svcall_device_data_transfer(void * args) MCU_ROOT_EXEC_CODE;
static int root_data_transfer_callback(void * context, const mcu_event_t * data) MCU_ROOT_CODE;
static void clear_device_action(const void * config, const devfs_device_t * device, int loc, int is_read);

int root_data_transfer_callback(void * context, const mcu_event_t * event){
	//activate all tasks that are blocked on this signal
	int i;
	int new_priority;
	svcall_device_data_transfer_t * args = (svcall_device_data_transfer_t*)context;

	new_priority = -1;
	if ( (uint32_t)args->async.tid < task_get_total() ){

		if ( scheduler_inuse_asserted(args->async.tid) ){
			if( event->o_events & MCU_EVENT_FLAG_CANCELED ){
				if( args->async.nbyte >= 0 ){
					args->async.nbyte = SYSFS_SET_RETURN(EAGAIN); //ignore any data transferred and return an error
				}
			}
		}

		if ( scheduler_inuse_asserted(args->async.tid) && !task_stopped_asserted(args->async.tid) ){ //check to see if the process terminated or stopped
			new_priority = task_get_priority( args->async.tid );
		}
	}

	//check to see if any tasks are waiting for this device -- is this even possible? Issue #148
	for(i = 1; i < task_get_total(); i++){
		if ( task_enabled(i) && scheduler_inuse_asserted(i) ){
			if ( sos_sched_table[i].block_object == (args->device + args->is_read) ){
				scheduler_root_assert_active(i, SCHEDULER_UNBLOCK_TRANSFER);
				if( !task_stopped_asserted(i) && (task_get_priority(i) > new_priority) ){
					new_priority = task_get_priority(i);
				}
			}
		}
	}

	if( args->is_read == ARGS_READ_READ ){
		args->is_read = ARGS_READ_DONE;
	}

	scheduler_root_update_on_wake(-1, new_priority);

	return 0;
}


void svcall_check_op_complete(void * args){
	CORTEXM_SVCALL_ENTER();
	root_check_op_complete(args);
}


void root_check_op_complete(void * args){
	svcall_device_data_transfer_t * p = (svcall_device_data_transfer_t*)args;

	if( p->is_read != ARGS_READ_DONE ){
		if ( p->result == 0 ){
			if ( p->async.nbyte >= 0 ){ //wait for the operation to complete or for data to arrive

				//checking the block object and sleeping have to happen atomically (driver could interrupt if it has a higher priority)
				cortexm_disable_interrupts();
				//Block waiting for the operation to complete or new data to be ready
				//if the interrupt has already fired the block_object will be zero already
				if( sos_sched_table[ task_get_current() ].block_object != 0 ){;
					//switch tasks until a signal becomes available
					scheduler_root_update_on_sleep();
				}
				cortexm_enable_interrupts();
			}
		} else {
			sos_sched_table[ task_get_current() ].block_object = 0;
			p->is_read = ARGS_READ_DONE;
		}
	}
}

void svcall_device_data_transfer(void * args){
	CORTEXM_SVCALL_ENTER();
	svcall_device_data_transfer_t * p = (svcall_device_data_transfer_t*)args;
	const devfs_device_t * dev = p->device;

	//check permissions on this device - IOCTL needs read/write access
	if( p->is_read ){
		if( sysfs_is_r_ok(dev->mode, dev->uid, SYSFS_GROUP) == 0 ){
			p->result = SYSFS_SET_RETURN(EPERM);
			return;
		}
	} else {
		if( sysfs_is_w_ok(dev->mode, dev->uid, SYSFS_GROUP) == 0 ){
			p->result = SYSFS_SET_RETURN(EPERM);
			return;
		}
	}


	//check async.buf and async.nbyte to ensure if belongs to the process
	//EPERM if it fails Issue #127
	if( task_validate_memory(p->async.buf, p->async.nbyte) < 0 ){
		p->result = SYSFS_SET_RETURN(EPERM);
	} else {
		//assume the operation is going to block
		sos_sched_table[ task_get_current() ].block_object = (void*)p->device + p->is_read;

		if ( p->is_read != 0 ){
			//Read operation
			p->result = dev->driver.read(&(dev->handle), &(p->async));
		} else {
			p->result = dev->driver.write(&(dev->handle), &(p->async));
		}
	}

	root_check_op_complete(args);
}

void clear_device_action(const void * config, const devfs_device_t * device, int loc, int is_read){
	mcu_action_t action;
	memset(&action, 0, sizeof(mcu_action_t));
	if( is_read ){
		action.o_events = MCU_EVENT_FLAG_DATA_READY;
	} else {
		action.o_events = MCU_EVENT_FLAG_WRITE_COMPLETE;
	}
	action.channel = loc;
	devfs_ioctl(config, (void*)device, I_MCU_SETACTION, &action);
}


int devfs_data_transfer(const void * config, const devfs_device_t * device, int flags, int loc, void * buf, int nbyte, int is_read){
	volatile svcall_device_data_transfer_t args;

	if ( nbyte == 0 ){
		return 0;
	}

	args.device = device;
	if( is_read ){
		args.is_read = ARGS_READ_READ;
	} else {
		args.is_read = ARGS_READ_WRITE;
	}
	args.async.loc = loc;
	args.async.flags = flags;
	args.async.buf = buf;
	args.async.handler.callback = root_data_transfer_callback;
	args.async.handler.context = (void*)&args;
	args.async.tid = task_get_current();
	int retry = 0;

	//privilege call for the operation
	do {

		args.result = -101010;
		args.async.nbyte = nbyte;

		//This transfers the data
		cortexm_svcall(svcall_device_data_transfer, (void*)&args);

		//We arrive here if the data is done transferring or there is no data to transfer and O_NONBLOCK is set
		//or if there was an error
		while( (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SIGNAL)
				 && ((volatile int)args.is_read != ARGS_READ_DONE) ){

			if( args.result == 0 ){
				//no data was transferred -- operation was interrupted by a signal
				clear_device_action(config, device, loc, args.is_read);
				return SYSFS_SET_RETURN(EINTR);
			}

			//check again if the op is complete
			cortexm_svcall(svcall_check_op_complete, (void*)&args);
		}

		if ( args.result == 0 ){
			//the operation happened asynchronously
			if ( args.async.nbyte > 0 ){
				//The operation has completed and transferred args.async.nbyte bytes
				args.result = args.async.nbyte;
			} else if ( args.async.nbyte == 0 ){
				//There was no data to read/write -- try again ??? not sure if this is right
				if (args.async.flags & O_NONBLOCK ){
					args.result = SYSFS_SET_RETURN(ENODATA);
				}
			} else if ( args.async.nbyte < 0 ){
				//there was an error executing the operation (or the operation was cancelled)
				args.result = args.async.nbyte;
			}
		} else if ( args.result < 0 ){
			//there was an error starting the operation (such as EAGAIN)

			//this is a rare/strange error where cortexm_svcall fails to run properly
			if( args.result == -101010 ){
				if( retry < 5 ){
					retry++;
					args.result = 0;
					mcu_debug_log_warning(MCU_DEBUG_DEVFS, "-101010 error");
				} else {
					args.result = SYSFS_SET_RETURN(EFAULT);
				}
			}
		}
	} while ( args.result == 0 );


	return args.result;
}

//used to execute any handler
int devfs_execute_event_handler(mcu_event_handler_t * handler, u32 o_events, void * data){
	int ret = 0;
	mcu_event_t event;
	if( handler->callback ){
		event.o_events = o_events;
		event.data = data;
		ret = handler->callback(handler->context, &event);
	}
	return ret;
}

void devfs_execute_cancel_handler(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte, u32 o_flags){
	devfs_execute_read_handler(transfer_handler, data, nbyte, o_flags | MCU_EVENT_FLAG_CANCELED);
	devfs_execute_write_handler(transfer_handler, data, nbyte, o_flags | MCU_EVENT_FLAG_CANCELED);
}

//this should be called when a read completes
int devfs_execute_read_handler(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte, u32 o_flags){
	if( transfer_handler->read ){
		devfs_async_t * async = transfer_handler->read;
		transfer_handler->read = 0;
		if( nbyte ){ async->nbyte = nbyte; }
		return devfs_execute_event_handler(&async->handler, o_flags, data);
	}
	return 0;
}

//this should be called when a write completes
int devfs_execute_write_handler(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte, u32 o_flags){
	if( transfer_handler->write ){
		devfs_async_t * async = transfer_handler->write;
		transfer_handler->write = 0;
		if( nbyte ){ async->nbyte = nbyte; }
		return devfs_execute_event_handler(
					&async->handler,
					o_flags,
					data);
	}
	return 0;
}


