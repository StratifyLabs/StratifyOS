// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <fcntl.h>

#include "mcu/mcu.h"

static int mcu_sync_io_complete(void *context, const mcu_event_t *data);

int mcu_sync_io_complete(void *context, const mcu_event_t *data) {
  MCU_UNUSED_ARGUMENT(data);
  int *done = context;
  *done = 1234567;
  // return 0 to delete the callback
  return 0;
}

int mcu_sync_io(
  const devfs_handle_t *handle,
  int (*func)(const devfs_handle_t *, devfs_async_t *),
  int loc,
  const void *buf,
  int nbyte,
  int flags) {
  devfs_async_t op;
  volatile int done;
  int ret;

  if (nbyte == 0) {
    return 0;
  }

  done = 0;
  op.buf_const = buf;
  op.loc = loc;
  op.flags = flags | O_RDWR;
  op.nbyte = nbyte;
  op.handler.context = (void *)&done;
  op.handler.callback = mcu_sync_io_complete;
  op.tid = 0;
  ret = func(handle, &op);

  if (ret == 0) {

    while (done != 1234567) { // cppcheck-suppress[knownConditionTrueFalse]
      ;
    }
    return op.nbyte;
  }
  return ret;
}
