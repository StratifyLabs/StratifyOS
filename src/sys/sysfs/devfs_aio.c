// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "devfs_local.h"

typedef struct {
  const devfs_device_t *device;
  struct aiocb *aiocbp;
  int read;
  int result;
} root_aio_transfer_t;

// static int data_transfer_callback(struct aiocb * aiocbp, const void * ignore);
static void svcall_device_data_transfer(void *args);

void svcall_device_data_transfer(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_aio_transfer_t *p = (root_aio_transfer_t *)args;

  cortexm_disable_interrupts(); // no switching until the transfer is started -- does
                                // Issue #130 change this
  // set the device callback for the read/write op
  if (p->read == 1) {
    // Read operation
    p->result =
      p->device->driver.read(&p->device->handle, (devfs_async_t *)&p->aiocbp->async);
  } else {
    p->result =
      p->device->driver.write(&p->device->handle, (devfs_async_t *)&p->aiocbp->async);
  }

  sos_sched_table[task_get_current()].block_object = NULL;

  cortexm_enable_interrupts();

  if (p->result == 0) {
    if (p->aiocbp->async.nbyte == 0) {
      // nothing was requested
      p->aiocbp->async.result = p->result;
      sysfs_aio_data_transfer_callback(p->aiocbp, 0);
      p->result = 0;
    } else if (p->aiocbp->async.nbyte > 0) {
      // AIO is in progress
    }
  } else if (p->result < 0) {
    // AIO was not started -- errno is set by the driver
    p->aiocbp->async.result = p->result;
    sysfs_aio_data_transfer_callback(p->aiocbp, 0);
  } else if (p->result > 0) {
    // The transfer happened synchronously -- call the callback manually
    p->aiocbp->async.result = p->result;
    sysfs_aio_data_transfer_callback(p->aiocbp, 0);
    p->result = 0;
  }
}

int devfs_aio_data_transfer(const devfs_device_t *device, struct aiocb *aiocbp) {
  root_aio_transfer_t args;
  args.device = device;
  args.aiocbp = aiocbp;
  if (aiocbp->aio_lio_opcode == LIO_READ) {
    args.read = 1;
  } else {
    args.read = 0;
  }

  args.aiocbp->async.loc = aiocbp->aio_offset;
  args.aiocbp->async.flags = 0; // never uses NON BLOCK because we are async
  args.aiocbp->async.nbyte = aiocbp->aio_nbytes;
  args.aiocbp->async.buf = (void *)aiocbp->aio_buf;
  args.aiocbp->async.tid = task_get_current();
  args.aiocbp->async.handler.callback = sysfs_aio_data_transfer_callback;
  args.aiocbp->async.handler.context = aiocbp;
  args.aiocbp->aio_nbytes = -1; // means status is in progress
  cortexm_svcall(svcall_device_data_transfer, &args);
  return args.result;
}
