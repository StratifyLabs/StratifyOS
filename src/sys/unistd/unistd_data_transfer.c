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

/*! \addtogroup UNISTD
 * @{
 */

/*! \file */

#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

#include "mcu/mcu.h"
#include "unistd_fs.h"
#include "unistd_flags.h"
#include "../sched/sched_flags.h"

#define ARGS_READ_WRITE 0
#define ARGS_READ_READ 1
#define ARGS_READ_DONE 2

typedef struct {
	const sysfs_t * fs;
	void * handle;
	devfs_async_t op;
	volatile int read;
	int ret;
} priv_device_data_transfer_t;

#if SINGLE_TASK != 0
volatile bool waiting;
#endif


static void priv_check_op_complete(void * args);
static void priv_device_data_transfer(void * args) MCU_PRIV_EXEC_CODE;
static int priv_data_transfer_callback(void * context, mcu_event_t * data) MCU_PRIV_CODE;
static int device_data_transfer(open_file_t * open_file, void * buf, int nbyte, int read);
static int get_mode(const sysfs_t* fs, void * handle);

int priv_data_transfer_callback(void * context, mcu_event_t * event){
	//activate all tasks that are blocked on this signal
	int i;
	int new_priority;
	priv_device_data_transfer_t * args = (priv_device_data_transfer_t*)context;

	if( event->o_events & MCU_EVENT_FLAG_CANCELED ){
		args->op.nbyte = -1; //ignore any data transferred and return an error
	}

	new_priority = -1;
	if ( (uint32_t)args->op.tid < task_get_total() ){
		if ( sched_inuse_asserted(args->op.tid) && !sched_stopped_asserted(args->op.tid) ){ //check to see if the process terminated or stopped
			new_priority = sos_sched_table[args->op.tid].priority;
		}
	}

	//check to see if any tasks are waiting for this device
	for(i = 1; i < task_get_total(); i++){
		if ( task_enabled(i) && sched_inuse_asserted(i) ){
			if ( sos_sched_table[i].block_object == (args->handle + args->read) ){
				sched_priv_assert_active(i, SCHED_UNBLOCK_TRANSFER);
				if( !sched_stopped_asserted(i) && (sos_sched_table[i].priority > new_priority) ){
					new_priority = sos_sched_table[i].priority;
				}
			}
		}
	}

	args->read = ARGS_READ_DONE;
	sched_priv_update_on_wake(new_priority);

	return 0;
}


void priv_check_op_complete(void * args){
	priv_device_data_transfer_t * p = (priv_device_data_transfer_t*)args;

	if( p->read != ARGS_READ_DONE ){
		if ( p->ret == 0 ){
			if ( (p->op.nbyte >= 0) //wait for the operation to complete or for data to arrive
			){
				//Block waiting for the operation to complete or new data to be ready
				sos_sched_table[ task_get_current() ].block_object = (void*)p->handle + p->read;
				//switch tasks until a signal becomes available
				sched_priv_update_on_sleep();
			}
		} else {
			p->read = ARGS_READ_DONE;
		}
	}
}

void priv_device_data_transfer(void * args){
	priv_device_data_transfer_t * p = (priv_device_data_transfer_t*)args;

	if ( p->read != 0 ){
		//Read operation
		p->ret = p->fs->read_async(p->fs->cfg, p->handle, &p->op);
	} else {
		p->ret = p->fs->write_async(p->fs->cfg, p->handle, &p->op);
	}

	priv_check_op_complete(args);

}

void unistd_clr_action(open_file_t * open_file){
	mcu_action_t action;
	memset(&action, 0, sizeof(mcu_action_t));
	u_ioctl(open_file, I_MCU_SETACTION, &action);
}


int u_read(open_file_t * open_file, void * buf, int nbyte){
	return device_data_transfer(open_file, buf, nbyte, 1);
}

int u_write(open_file_t * open_file, const void * buf, int nbyte){
	return device_data_transfer(open_file, (void*)buf, nbyte, 0);
}


int device_data_transfer(open_file_t * open_file, void * buf, int nbyte, int read){
	int tmp;
	int mode;
	volatile priv_device_data_transfer_t args;

	if ( nbyte == 0 ){
		return 0;
	}

	args.fs = (const sysfs_t*)open_file->fs;
	args.handle = (devfs_device_t *)open_file->handle;
	if( read ){
		args.read = ARGS_READ_READ;
	} else {
		args.read = ARGS_READ_WRITE;
	}
	args.op.loc = open_file->loc;
	args.op.flags = open_file->flags;
	args.op.buf = buf;
	args.op.handler.callback = priv_data_transfer_callback;
	args.op.handler.context = (void*)&args;
	args.op.tid = task_get_current();

	if ( (mode = get_mode(args.fs, args.handle)) < 0 ){
		return -1;
	}

	tmp = 0;

	//privilege call for the operation
	do {

		args.ret = -101010;
		args.op.nbyte = nbyte;

		//This transfers the data
		mcu_core_privcall(priv_device_data_transfer, (void*)&args);

		//We arrive here if the data is done transferring or there is no data to transfer and O_NONBLOCK is set
		//or if there was an error
		while( (sched_get_unblock_type(task_get_current()) == SCHED_UNBLOCK_SIGNAL)
				&& ((volatile int)args.read != ARGS_READ_DONE) ){

			if( (args.ret == 0) && (args.op.nbyte == 0) ){
				//no data was transferred
				unistd_clr_action(open_file);
				errno = EINTR;
				//return the number of bytes transferred
				return -1;
			}

			//check again if the op is complete
			mcu_core_privcall(priv_check_op_complete, (void*)&args);
		}


		if ( args.ret > 0 ){
			//The operation happened synchronously
			tmp = args.ret;
			break;
		} else if ( args.ret == 0 ){
			//the operation happened asynchronously
			if ( args.op.nbyte > 0 ){
				//The operation has completed and transferred args.op.nbyte bytes
				tmp = args.op.nbyte;
				break;
			} else if ( args.op.nbyte == 0 ){
				//There was no data to read/write -- try again
				if (args.op.flags & O_NONBLOCK ){
					errno = ENODATA;
					return -1;
				}

			} else if ( args.op.nbyte < 0 ){
				//there was an error executing the operation (or the operation was cancelled)
				return -1;
			}
		} else if ( args.ret < 0 ){
			//there was an error starting the operation (such as EAGAIN)
			if( args.ret == -101010 ){
				errno = ENXIO; //this is a rare/strange error where mcu_core_privcall fails to run properly
			}
			return args.ret;
		}
	} while ( args.ret == 0 );


	if ( ((mode & S_IFMT) != S_IFCHR) && (tmp > 0) ){
		open_file->loc += tmp;
	}

	return tmp;
}

int get_mode(const sysfs_t* fs, void * handle){
	struct stat st;
	if ( fs->fstat(fs->cfg, handle, &st) < 0){
		return -1;
	}
	return st.st_mode;
}

/*! @} */


