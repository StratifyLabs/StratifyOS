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

#include "devfs_local.h"

typedef struct {
	const devfs_device_t * device;
	struct aiocb * aiocbp;
	int read;
	int ret;
} root_aio_transfer_t;

//static int data_transfer_callback(struct aiocb * aiocbp, const void * ignore);
static void root_device_data_transfer(void * args);

void root_device_data_transfer(void * args){
	root_aio_transfer_t * p = (root_aio_transfer_t*)args;

    cortexm_disable_interrupts(); //no switching until the transfer is started -- does Issue #130 change this
	//set the device callback for the read/write op
	if ( p->read == 1 ){
		//Read operation
		p->ret = p->device->driver.read(&p->device->handle, &p->aiocbp->op);
	} else {
		p->ret = p->device->driver.write(&p->device->handle, &p->aiocbp->op);
	}

	sos_sched_table[task_get_current()].block_object = NULL;

    cortexm_enable_interrupts();

	if ( p->ret == 0 ){
		if( p->aiocbp->op.nbyte > 0 ){
			//AIO is in progress
		}
	} else if ( p->ret < 0 ){
		//AIO was not started -- errno is set by the driver
        //p->ret = -1;
	} else if ( p->ret > 0 ){
		//The transfer happened synchronously -- call the callback manually
		sysfs_aio_data_transfer_callback(p->aiocbp, 0);
		p->ret = 0;
	}
}

int devfs_aio_data_transfer(const devfs_device_t * device, struct aiocb * aiocbp){
	root_aio_transfer_t args;
	args.device = device;
	args.aiocbp = aiocbp;
	if ( aiocbp->aio_lio_opcode == LIO_READ ){
		args.read = 1;
	} else {
		args.read = 0;
	}
	args.aiocbp->op.loc = aiocbp->aio_offset;
	args.aiocbp->op.flags = 0; //this is never a blocking call
	args.aiocbp->op.nbyte = aiocbp->aio_nbytes;
	args.aiocbp->op.buf = (void*)aiocbp->aio_buf;
	args.aiocbp->op.tid = task_get_current();
	args.aiocbp->op.handler.callback = sysfs_aio_data_transfer_callback;
	args.aiocbp->op.handler.context = aiocbp;
	args.aiocbp->aio_nbytes = -1; //means status is in progress
	cortexm_svcall(root_device_data_transfer, &args);
	return args.ret;
}
