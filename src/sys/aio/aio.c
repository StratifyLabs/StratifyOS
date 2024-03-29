// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup aio
 *
 * @{
 *
 */

/*! \file */


#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "../signal/sig_local.h"
#include "../unistd/unistd_local.h"
#include "cortexm/cortexm.h"
#include "sos/fs/sysfs.h"

#include "../scheduler/scheduler_root.h"
#include "../scheduler/scheduler_timing.h"


/*! \cond */
static void svcall_suspend(void *args) MCU_ROOT_EXEC_CODE;
static int suspend(
  struct aiocb *const list[],
  int nent,
  const struct timespec *timeout,
  u8 block_on_all);
static int data_transfer(struct aiocb *aiocbp);
/*! \endcond */

/*! \details This function is not supported this version.
 *
 * \return -1 with errno set to ENOTSUP
 */
int aio_cancel(
  int fildes /*! the file descriptor */,
  struct aiocb *aiocbp /*! a pointer to the AIO data structure */) {

  // this needs a special ioctl request to cancel current operations -- use MCU_SET_ACTION
  mcu_action_t action;
  memset(&action, 0, sizeof(mcu_action_t));
  if (aiocbp->aio_lio_opcode == LIO_READ) {
    action.o_events = MCU_EVENT_FLAG_DATA_READY;
  } else {
    action.o_events = MCU_EVENT_FLAG_WRITE_COMPLETE;
  }
  action.channel = aiocbp->aio_offset;
  return ioctl(fildes, I_MCU_SETACTION, &action);
}

/*! \details This function gets the error value for \a aiocbp.
 *
 * \return The error value for the operation or zero for no errors:
 *  - EINPROGRESS:  The operation is still in progress
 *  - Other interface specific error
 */
int aio_error(const struct aiocb *aiocbp /*! a pointer to the AIO data struture */) {
  if ((volatile void *)aiocbp->async.buf != NULL) {
    return EINPROGRESS;
  } else {
    // this is where the error value is stored in case of failure
    return aiocbp->async.result;
  }
}

/*! \details This function is not supported this version.
 * \return -1 with errno set to ENOTSUP
 */
int aio_fsync(int op, struct aiocb *aiocbp) {
  MCU_UNUSED_ARGUMENT(op);
  MCU_UNUSED_ARGUMENT(aiocbp);
  // all data is already synced
  errno = ENOTSUP;
  return -1;
}

/*! \details The function initiates an asynchronous read using the data specified by \a
 * aiocbp.
 *
 * \return 0 on success or -1 with errno (see \ref errno) set to:
 *  - EIO:  failed to start the transfer
 */
int aio_read(struct aiocb *aiocbp /*! a pointer to the AIO data struture */) {
  aiocbp->aio_lio_opcode = LIO_READ;
  return data_transfer(aiocbp);
}

/*! \details The function initiates an asynchronous write using the data specified by \a
 * aiocbp.
 *
 * \return 0 on success or -1 with errno (see \ref errno) set to:
 *  - EIO:  failed to start the transfer
 */
int aio_write(struct aiocb *aiocbp /*! a pointer to the AIO data struture */) {
  aiocbp->aio_lio_opcode = LIO_WRITE;
  return data_transfer(aiocbp);
}

/*! \details This function returns the number of bytes read/written for the asynchronous
 * operation.
 *
 * \return The number of bytes transferred or -1 if there was an error
 * (or if the operation is not yet complete).  \ref aio_error() can be used to determine
 * the error.
 */
ssize_t aio_return(struct aiocb *aiocbp /*! a pointer to the AIO data struture */) {
  if (aiocbp->async.buf != NULL) {
    return -1;
  } else {
    return aiocbp->aio_nbytes; // this is the number of bytes that was read or written NOLINT(cppcoreguidelines-narrowing-conversions)
  }
}

int suspend(
  struct aiocb *const list[],
  int nent,
  const struct timespec *timeout,
  u8 block_on_all) {
  sysfs_aio_suspend_t args;

  // suspend until an AIO operation completes or until timeout is exceeded
  args.list = list;
  args.nent = nent;
  args.block_on_all = block_on_all; // only block on one or block on all
  scheduler_timing_convert_timespec(&args.abs_timeout, timeout);
  cortexm_svcall(svcall_suspend, &args);

  if (args.nent == -1) {
    return 0; // one of the AIO's in the list has already completed
  }

  // Check the unblock type
  if (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SLEEP) {
    errno = EAGAIN;
    return -1;
  } else if (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SIGNAL) {
    // check to see if a signal interrupted the sleeping period
    errno = EINTR;
    return -1;
  }

  return 0;
}

/*! \details This function will suspend the currently executing thread until an AIO
 * operation in \a list completes or until the \a timeout value is surpassed.  If \a
 * timeout is NULL, it is ignored.
 *
 * \return 0 on success or -1 with errno (see \ref errno) set to:
 *  - EAGAIN:  \a timeout was exceeded before any operations completed.
 *  - EINTR:  the thread received a signal before any operations completed.
 */
int aio_suspend(
  struct aiocb *const list[] /*! a list of AIO transfer structures */,
  int nent /*! the number of transfer in \a list */,
  const struct timespec *timeout /*! the absolute timeout value */) {
  return suspend(list, nent, timeout, 0);
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
 * - ENOTSUP: \a mode is LIO_NOWAIT and \a sig is not null or sigev_notify is not set to
 * SIGEV_NONE
 * - EINVAL:  \a mode is not set to LIO_NOWAIT or LIO_WAIT
 */
int lio_listio(
  int mode /*! The mode:  \a LIO_WAIT or \a LIO_NOWAIT */,
  struct aiocb *const list[] /*! The list of AIO transfers */,
  int nent /*! The number of transfers in \a list */,
  struct sigevent *sig /*! The sigevent structure */) {
  int i;

  switch (mode) {
  case LIO_NOWAIT:
    if (sig != NULL) {
      for (i = 0; i < nent; i++) {
        if (list[i] != 0) {
          // error check sigevent
          memcpy(&list[i]->aio_sigevent, sig, sizeof(struct sigevent));
        }
      }
    }

    // no break
  case LIO_WAIT:
    break;
  default:
    errno = EINVAL;
    return -1;
  }

  for (i = 0; i < nent; i++) {
    // start all of the operations in the list
    if (list[i] != NULL) { // ignore NULL entries
      data_transfer(list[i]);
    }
  }

  if (mode == LIO_WAIT) {
    return suspend(list, nent, NULL, 1);
  }

  return 0;
}

/*! \cond */
int data_transfer(struct aiocb *aiocbp) {
  int fildes;
  sysfs_file_t *file;

  fildes = u_fildes_is_bad(aiocbp->aio_fildes);
  if (fildes < 0) {
    // check to see if fildes is a socket
    errno = EBADF;
    return -1;
  }
  aiocbp->aio_fildes = fildes;
  file = get_open_file(fildes);
  return sysfs_file_aio(file, aiocbp);
}

void svcall_suspend(void *args) {
  CORTEXM_SVCALL_ENTER();
  int i;
  u8 is_suspend;
  sysfs_aio_suspend_t *p = (sysfs_aio_suspend_t *)args;
  // disable interrupts
  //! \todo See if an operation is in progress
  if (p->block_on_all == 0) {
    is_suspend = 1;
  } else {
    is_suspend = 0;
  }
  cortexm_disable_interrupts(); // no switching until the transfer is started
  for (i = 0; i < p->nent; i++) {
    if (p->list[i] != NULL) {

      // first check to see if we block on aio suspend (if anything is complete don't
      // block)
      // if op.buf is NULL the operation is complete
      if ((p->list[i]->async.buf == NULL) && (p->block_on_all == 0)) {
        is_suspend = 0;
        break;
      }

      // now check to see if we block on listio suspend (if anything is incomplete block)
      if ((p->list[i]->async.buf != NULL) && (p->block_on_all == 1)) {
        is_suspend = 1;
      }
    }
  }

  if (is_suspend == 1) {
    scheduler_root_assert_aiosuspend(task_get_current());
    scheduler_timing_root_timedblock(args, &p->abs_timeout);
  } else {
    p->nent = -1;
  }

  // enable interrupts
  cortexm_enable_interrupts();
}

int sysfs_aio_data_transfer_callback(void *context, const mcu_event_t *event) {
  MCU_UNUSED_ARGUMENT(event);
  struct aiocb *aiocbp;
  aiocbp = context;
  aiocbp->async.buf = NULL;
  int tid = aiocbp->async.tid;

  if (aiocbp->async.result < 0) {
    aiocbp->aio_nbytes = SYSFS_GET_RETURN(aiocbp->async.result); // given for aio_return
    aiocbp->async.result = SYSFS_GET_RETURN_ERRNO(aiocbp->async.result);
  } else {
    aiocbp->aio_nbytes = aiocbp->async.result; // given for aio_return
    aiocbp->async.result = 0;                  // given for aio_error()
  }

  if (tid >= task_get_total()) {
    // This is not a valid task id
    return 0;
  }

  if (task_enabled(tid)) { // if task is no longer enabled (don't do anything)

    // Check to see if the thread is suspended on aio -- the block object is a list of
    // aiocb -- check if aiocbp is in list
    if (scheduler_aiosuspend_asserted(tid)) {
      sysfs_aio_suspend_t *p = (sysfs_aio_suspend_t *)sos_sched_table[tid].block_object;

      if (p->block_on_all == 0) { // don't block on all -- unblock on any
        for (int i = 0; i < p->nent; i++) {
          if (aiocbp == p->list[i]) { // If this is true the thread is blocked on the
                                      // operation that is currently completing
            scheduler_root_assert_active(tid, SCHEDULER_UNBLOCK_AIO);
            scheduler_root_update_on_wake(tid, task_get_priority(tid));
            break;
          }
        }
      } else {
        u8 wakeup = 1;
        for (int i = 0; i < p->nent; i++) {
          if (p->list[i]->async.buf != NULL) { // operation is not complete
            // don't wakeup because this operation is not complete
            wakeup = 0;
          }
        }

        if (wakeup) {
          scheduler_root_assert_active(tid, SCHEDULER_UNBLOCK_AIO);
          scheduler_root_update_on_wake(tid, task_get_priority(tid));
        }
      }
    }

    // Need to send an asynchronous notification if a lio_listio call was made -- no limit
    // to number of lio_listio calls
    if (aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL) {
      // send a signal
      signal_root_send(
        0, tid, aiocbp->aio_sigevent.sigev_signo, SI_ASYNCIO,
        aiocbp->aio_sigevent.sigev_value.sival_int, 1);
    }

    // This needs to check if all operations in a list have completed and then use
    // SIGEV_NONE, SIGEV_SIGNAL, or SIGEV_THREAD to notify
  }

  return 0;
}
/*! \endcond */

/*! @} */
