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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */


/*! \addtogroup aio
 *
 * @{
 *
 */

/*! \file */

#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "cortexm/cortexm.h"
#include "mcu/debug.h"
#include "sos/fs/sysfs.h"
#include "../scheduler/scheduler_local.h"
#include "../unistd/unistd_local.h"
#include "../signal/sig_local.h"


/*! \cond */
static void root_suspend(void * args) MCU_ROOT_EXEC_CODE;
static int suspend(struct aiocb *const list[], int nent, const struct timespec * timeout, bool block_on_all);
static int data_transfer(struct aiocb * aiocbp);

typedef struct {
	const devfs_device_t * device;
	struct aiocb * aiocbp;
	int read;
	int result;
} root_aio_transfer_t;
/*! \endcond */


/*! \details This function is not supported this version.
 *
 * \return -1 with errno set to ENOTSUP
 */
int aio_cancel(int fildes /*! the file descriptor */,
					struct aiocb * aiocbp /*! a pointer to the AIO data structure */){

	//this needs a special ioctl request to cancel current operations -- use MCU_SET_ACTION

	errno = ENOTSUP;
	return -1;
}

/*! \details This function gets the error value for \a aiocbp.
 *
 * \return The error value for the operation or zero for no errors:
 *  - EINPROGRESS:  The operation is still in progress
 *  - Other interface specific error
 */
int aio_error(const struct aiocb * aiocbp /*! a pointer to the AIO data struture */){
	if ( (volatile void *)aiocbp->async.buf != NULL ){
		return EINPROGRESS;
	} else {
		return aiocbp->async.nbyte; //this is where the error value is stored in case of failure
	}
}

/*! \details This function is not supported this version.
 * \return -1 with errno set to ENOTSUP
 */
int aio_fsync(int op, struct aiocb * aiocbp){
	//all data is already synced
	errno = ENOTSUP;
	return -1;
}

/*! \details The function initiates an asynchronous read using the data specified by \a aiocbp.
 *
 * \return 0 on success or -1 with errno (see \ref errno) set to:
 *  - EIO:  failed to start the transfer
 */
int aio_read(struct aiocb * aiocbp /*! a pointer to the AIO data struture */){
	aiocbp->aio_lio_opcode = LIO_READ;
	return data_transfer(aiocbp);
}

/*! \details The function initiates an asynchronous write using the data specified by \a aiocbp.
 *
 * \return 0 on success or -1 with errno (see \ref errno) set to:
 *  - EIO:  failed to start the transfer
 */
int aio_write(struct aiocb * aiocbp /*! a pointer to the AIO data struture */){
	aiocbp->aio_lio_opcode = LIO_WRITE;
	return data_transfer(aiocbp);
}

/*! \details This function returns the number of bytes read/written for the asynchronous operation.
 *
 * \return The number of bytes transferred or -1 if there was an error
 * (or if the operation is not yet complete).  \ref aio_error() can be used to determine the error.
 */
ssize_t aio_return(struct aiocb * aiocbp /*! a pointer to the AIO data struture */){
	if ( aiocbp->async.buf != NULL ){
		return -1;
	} else {
		return aiocbp->aio_nbytes; //this is the number of bytes that was read or written
	}
}



int suspend(struct aiocb *const list[], int nent, const struct timespec * timeout, bool block_on_all){
	sysfs_aio_suspend_t args;

	//suspend until an AIO operation completes or until timeout is exceeded
	args.list = list;
	args.nent = nent;
	args.block_on_all = block_on_all; //only block on one or block on all
	scheduler_timing_convert_timespec(&args.abs_timeout, timeout);
	cortexm_svcall(root_suspend, &args);

	if( args.nent == -1 ){
		return 0; //one of the AIO's in the list has already completed
	}

	//Check the unblock type
	if ( scheduler_unblock_type( task_get_current() ) == SCHEDULER_UNBLOCK_SLEEP ){
		errno = EAGAIN;
		return -1;
	} else if ( scheduler_unblock_type( task_get_current() ) == SCHEDULER_UNBLOCK_SIGNAL ){
		//check to see if a signal interrupted the sleeping period
		errno = EINTR;
		return -1;
	}

	return args.result;
}

/*! \details This function will suspend the currently executing thread until an AIO operation
 * in \a list completes or until the \a timeout value is surpassed.  If \a timeout is NULL, it is
 * ignored.
 *
 * \return 0 on success or -1 with errno (see \ref errno) set to:
 *  - EAGAIN:  \a timeout was exceeded before any operations completed.
 *  - EINTR:  the thread received a signal before any operations completed.
 */
int aio_suspend(struct aiocb *const list[] /*! a list of AIO transfer structures */,
					 int nent /*! the number of transfer in \a list */,
					 const struct timespec * timeout /*! the absolute timeout value */){
	return suspend(list, nent, timeout, false);
}

/*! \details This function initiates a list of asynchronous transfers.
 *
 * \note Asynchronous notification is not supported in this version.  \a ENOTSUP
 * is returned if an attempt is made to invoke an asynchronous notification.
 *
 * \note This function is experimental in this version.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINTR:  the thread received an signal before completed all transactions
 * - ENOTSUP: \a mode is LIO_NOWAIT and \a sig is not null or sigev_notify is not set to SIGEV_NONE
 * - EINVAL:  \a mode is not set to LIO_NOWAIT or LIO_WAIT
 */
int lio_listio(int mode /*! The mode:  \a LIO_WAIT or \a LIO_NOWAIT */,
					struct aiocb * const list[] /*! The list of AIO transfers */,
					int nent /*! The number of transfers in \a list */,
					struct sigevent * sig /*! The sigevent structure */){
	int i;

	switch(mode){
		case LIO_NOWAIT:
			if ( sig != NULL ){
				for(i=0; i < nent; i++){
					if( list[i] != 0 ){
						//error check sigevent
						memcpy(&list[i]->aio_sigevent, sig, sizeof(struct sigevent));
					}
				}
			}

			//no break
		case LIO_WAIT:
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	for(i=0; i < nent; i++){
		//start all of the operations in the list
		if( list[i] != NULL ){  //ignore NULL entries
			data_transfer(list[i]);
		}
	}

	if ( mode == LIO_WAIT ){
		return suspend(list, nent, NULL, true);
	}

	return 0;
}

/*! \cond */
int data_transfer(struct aiocb * aiocbp){
	int fildes;
	sysfs_file_t * file;

	fildes = u_fildes_is_bad(aiocbp->aio_fildes);
	if ( fildes < 0 ){
		//check to see if fildes is a socket
		errno = EBADF;
		return -1;
	}
	aiocbp->aio_fildes = fildes;
	file = get_open_file(fildes);
	return sysfs_file_aio(file, aiocbp);
}


void root_suspend(void * args){
	int i;
	bool suspend;
	sysfs_aio_suspend_t * p = (sysfs_aio_suspend_t*)args;
	//disable interrupts
	//! \todo See if an operation is in progress
	if ( p->block_on_all == false ){
		suspend = true;
	} else {
		suspend = false;
	}
	cortexm_disable_interrupts(); //no switching until the transfer is started
	for(i = 0; i < p->nent; i++ ){
		if (p->list[i] != NULL ){

			//first check to see if we block on aio suspend (if anything is complete don't block)
			if ( (p->list[i]->async.buf == NULL) && (p->block_on_all == false) ){ //if op.buf is NULL the operation is complete
				suspend = false;
				break;
			}

			//now check to see if we block on listio suspend (if anything is incomplete block)
			if ( (p->list[i]->async.buf != NULL) && (p->block_on_all == true) ){
				suspend = true;
			}

		}
	}

	if ( suspend == true ){
		scheduler_root_assert_aiosuspend(task_get_current());
		scheduler_timing_root_timedblock(args, &p->abs_timeout);
	} else {
		p->nent = -1;
	}

	//enable interrupts
	cortexm_enable_interrupts();

}

int sysfs_aio_data_transfer_callback(void * context, const mcu_event_t * event){
	struct aiocb * aiocbp;
	unsigned int tid;
	int i;
	bool wakeup;
	aiocbp = context;
	sysfs_aio_suspend_t * p;
	aiocbp->aio_nbytes = aiocbp->async.nbyte;
	aiocbp->async.buf = NULL;
	if( aiocbp->async.nbyte < 0 ){
		aiocbp->async.nbyte = SYSFS_GET_RETURN_ERRNO(aiocbp->async.nbyte);
	} else {
		aiocbp->async.nbyte = 0;
	}

	//Check to see if the thread is suspended on aio -- the block object is a list of aiocb -- check if aiocbp is in list
	tid = aiocbp->async.tid;
	if( tid >= task_get_total() ){
		//This is not a valid task id
		return 0;
	}

	if( task_enabled(tid) ){ //if task is no longer enabled (don't do anything)
		if ( scheduler_aiosuspend_asserted(tid) ){
			p = (sysfs_aio_suspend_t*)sos_sched_table[tid].block_object;

			if ( p->block_on_all == false ){
				for(i=0; i < p->nent; i++){
					if ( aiocbp == p->list[i] ){ //If this is true the thread is blocked on the operation that is currently completing
						scheduler_root_assert_active(tid, SCHEDULER_UNBLOCK_AIO);
						scheduler_root_update_on_wake(tid, task_get_priority(tid) );
						break;
					}
				}
			} else {
				wakeup = true;
				for(i=0; i < p->nent; i++){
					if ( p->list[i]->async.buf != NULL ){ //operation is not complete
						//don't wakeup because this operation is not complete
						wakeup = false;
					}
				}

				if( wakeup == true ){
					scheduler_root_assert_active(tid, SCHEDULER_UNBLOCK_AIO);
					scheduler_root_update_on_wake(tid, task_get_priority(tid));
				}
			}
		}

		//Need to send an asynchronous notification if a lio_listio call was made -- no limit to number of lio_listio calls
		if( aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL ){
			//send a signal
			signal_root_send(0,
								  tid,
								  aiocbp->aio_sigevent.sigev_signo,
								  SI_ASYNCIO,
								  aiocbp->aio_sigevent.sigev_value.sival_int,
								  1);
		}

		//This needs to check if all operations in a list have complete and then use SIGEV_NONE, SIGEV_SIGNAL, or SIGEV_THREAD to notify
		return 0;
	}

	return 0;
}
/*! \endcond */


/*! @} */

