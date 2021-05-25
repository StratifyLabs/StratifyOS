// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "sos/debug.h"
#include <errno.h>
#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "../scheduler/scheduler_local.h"
#include "devfs_local.h"
#include "sos/fs/sysfs.h"

#define ARGS_TRANSFER_WRITE 0
#define ARGS_TRANSFER_READ 1
#define ARGS_TRANSFER_DONE 2

typedef struct {
  const void *device;
  devfs_async_t async;
  volatile int transfer_type;
  int result;
} svcall_device_data_transfer_t;

// static void svcall_check_op_complete(void * args);
static void root_check_op_complete(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_device_data_transfer(void *args) MCU_ROOT_EXEC_CODE;
static int
root_data_transfer_callback(void *context, const mcu_event_t *data) MCU_ROOT_CODE;
static void clear_device_action(
  const void *config,
  const devfs_device_t *device,
  int loc,
  int is_read);

int root_data_transfer_callback(void *context, const mcu_event_t *event) {
  // activate all tasks that are blocked on this signal
  // int i;
  svcall_device_data_transfer_t *args = (svcall_device_data_transfer_t *)context;

  int new_priority = -1;
  if ((uint32_t)args->async.tid < task_get_total()) {

    // make sure the ID is still in use and hasn't crashed since the transfer started
    if (scheduler_inuse_asserted(args->async.tid)) {
      if (event->o_events & MCU_EVENT_FLAG_CANCELED) {
        // ignore any data transferred and return an error
        if (args->async.result >= 0) {
          args->async.result = SYSFS_SET_RETURN(EAGAIN);
        }
      }

      // check to see if the process is halted temporarily
      if (!task_stopped_asserted(args->async.tid)) {
        new_priority = task_get_priority(args->async.tid);
        // remove the loop below and just use this line?
        scheduler_root_assert_active(args->async.tid, SCHEDULER_UNBLOCK_TRANSFER);
      }
    }
  }

  // check to see if any tasks are waiting for this device -- is this even possible? Issue
  // #148
#if 0
  for (i = 1; i < task_get_total(); i++) {
    if (task_enabled(i) && scheduler_inuse_asserted(i)) {
      if (sos_sched_table[i].block_object == (args->device + args->transfer_type)) {
        scheduler_root_assert_active(i, SCHEDULER_UNBLOCK_TRANSFER);
        if (!task_stopped_asserted(i) && (task_get_priority(i) > new_priority)) {
          new_priority = task_get_priority(i);
        }
      }
    }
  }
#endif
  args->transfer_type = ARGS_TRANSFER_DONE;
  scheduler_root_update_on_wake(-1, new_priority);
  return 0;
}

void svcall_device_data_transfer(void *args) {
  CORTEXM_SVCALL_ENTER();
  svcall_device_data_transfer_t *p = (svcall_device_data_transfer_t *)args;
  const devfs_device_t *dev = p->device;

  // check permissions on this device
  if (p->transfer_type == ARGS_TRANSFER_READ) {
    if (sysfs_is_r_ok(dev->mode, dev->uid, SYSFS_GROUP) == 0) {
      p->result = SYSFS_SET_RETURN(EPERM);
      return;
    }
  } else {
    if (sysfs_is_w_ok(dev->mode, dev->uid, SYSFS_GROUP) == 0) {
      p->result = SYSFS_SET_RETURN(EPERM);
      return;
    }
  }

  // check async.buf and async.nbyte to ensure if belongs to the process
  // EPERM if it fails Issue #127
  if (task_validate_memory(p->async.buf, p->async.nbyte) < 0) {
    p->result = SYSFS_SET_RETURN(EPERM);
    return;
  }

  // assume the operation is going to block
  sos_sched_table[task_get_current()].block_object = (u8 *)p->device + p->transfer_type;
  if (p->transfer_type == ARGS_TRANSFER_READ) {
    p->result = dev->driver.read(&(dev->handle), &(p->async));
  } else {
    p->result = dev->driver.write(&(dev->handle), &(p->async));
  }

  root_check_op_complete(args);
}

// void svcall_check_op_complete(void * args){
//	CORTEXM_SVCALL_ENTER();
//	root_check_op_complete(args);
//}

void root_check_op_complete(void *args) {
  svcall_device_data_transfer_t *p = (svcall_device_data_transfer_t *)args;

  if (p->transfer_type != ARGS_TRANSFER_DONE) {
    // callback has not been executed yet
    if ((p->result == 0) && (p->async.nbyte >= 0)) {
      // checking the block object and sleeping have to happen
      // atomically (driver could interrupt if it has a higher priority)
      cortexm_disable_interrupts();
      // Block waiting for the operation to complete or new data to be ready
      // if the interrupt has already fired the block_object will be zero already
      if (sos_sched_table[task_get_current()].block_object != 0) {
        ;
        // switch tasks until a signal becomes available
        scheduler_root_update_on_sleep();
      }
      cortexm_enable_interrupts();
    } else {
      // p->result is not zero OR nbyte is less than zero-> means:
      // operation happened sychronously -- no need to block
      sos_sched_table[task_get_current()].block_object = 0;
      p->transfer_type = ARGS_TRANSFER_DONE;
    }
  }
}

void clear_device_action(
  const void *config,
  const devfs_device_t *device,
  int loc,
  int is_read) {
  mcu_action_t action;
  memset(&action, 0, sizeof(mcu_action_t));
  if (is_read) {
    action.o_events = MCU_EVENT_FLAG_DATA_READY;
  } else {
    action.o_events = MCU_EVENT_FLAG_WRITE_COMPLETE;
  }
  action.channel = loc;
  devfs_ioctl(config, (void *)device, I_MCU_SETACTION, &action);
}

int devfs_data_transfer(
  const void *config,
  const devfs_device_t *device,
  int flags,
  int loc,
  void *buf,
  int nbyte,
  int is_read) {
  volatile svcall_device_data_transfer_t args;

  if (nbyte == 0) {
    // this checks for permissions and other errors
    return 0;
  }

  args.device = device;
  args.async.loc = loc;
  args.async.flags = flags;
  args.async.buf = buf;
  args.async.handler.callback = root_data_transfer_callback;
  args.async.handler.context = (void *)&args;
  args.async.tid = task_get_current();
  int retry = 0;

  // privilege call for the operation
  do {

    if (is_read) {
      args.transfer_type = ARGS_TRANSFER_READ;
    } else {
      args.transfer_type = ARGS_TRANSFER_WRITE;
    }
    args.result = -101010;
    args.async.nbyte = nbyte;
    args.async.result = 0;

    // This transfers the data
    cortexm_svcall(svcall_device_data_transfer, (void *)&args);

    // We arrive here if
    // the data is done transferring
    // OR there is no data to transfer and O_NONBLOCK is set
    // OR nbyte and zero and error checking was performed
    // OR if there was an error
    // OR transfer was interrupted by a signal
    if (
      (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SIGNAL)
      && (args.result == 0) && (args.transfer_type != ARGS_TRANSFER_DONE)) {

      clear_device_action(config, device, loc, args.transfer_type == ARGS_TRANSFER_READ);
      return SYSFS_SET_RETURN(EINTR);
    }

    if (args.result == 0) {
      // the operation happened asynchronously
      if (args.async.result != 0) {
        // The operation has completed and transferred args.async.nbyte bytes
        // OR there was an error executing the operation (or the operation was cancelled)
        args.result = args.async.result;
      } else {
        // result and nbyte are zero
        // this will happen if the driver assigned nbyte to zero because it wants
        // to be invoked again in order to, for example, empty a buffer
        sos_debug_log_warning(
          SOS_DEBUG_DEVFS, "device %s assigned 0 to nbyte", device->name);
        if (retry < 5) {
          retry++;
        } else {
          args.result = SYSFS_SET_RETURN(ELOOP);
        }
      }
    } else if (args.result < 0) {
      // there was an error starting the operation (such as EAGAIN)

      // this is a rare/strange error where cortexm_svcall fails to run properly
      if (args.result == -101010) {
        if (retry < 5) {
          retry++;
          args.result = 0;
          sos_debug_log_warning(SOS_DEBUG_DEVFS, "-101010 error on %s", device->name);
        } else {
          args.result = SYSFS_SET_RETURN(EFAULT);
        }
      }
    }
  } while (args.result == 0);

  return args.result;
}
