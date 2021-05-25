#include "sos/fs/devfs.h"

// used to execute any handler
int devfs_execute_event_handler(mcu_event_handler_t *handler, u32 o_events, void *data) {
  int ret = 0;
  mcu_event_t event;
  if (handler->callback) {
    event.o_events = o_events;
    event.data = data;
    ret = handler->callback(handler->context, &event);
  }
  return ret;
}

void devfs_execute_cancel_handler(
  devfs_transfer_handler_t *transfer_handler,
  void *data,
  int nbyte,
  u32 o_flags) {
  devfs_execute_read_handler(
    transfer_handler, data, nbyte, o_flags | MCU_EVENT_FLAG_CANCELED);
  devfs_execute_write_handler(
    transfer_handler, data, nbyte, o_flags | MCU_EVENT_FLAG_CANCELED);
}

// this should be called when a read completes
int devfs_execute_read_handler(
  devfs_transfer_handler_t *transfer_handler,
  void *data,
  int nbyte,
  u32 o_flags) {
  if (transfer_handler->read) {
    devfs_async_t *async = transfer_handler->read;
    transfer_handler->read = NULL;
    if (nbyte == 0) {
      async->result = async->nbyte;
    } else {
      async->result = nbyte;
    }
    return devfs_execute_event_handler(&async->handler, o_flags, data);
  }
  return 0;
}

// this should be called when a write completes
int devfs_execute_write_handler(
  devfs_transfer_handler_t *transfer_handler,
  void *data,
  int nbyte,
  u32 o_flags) {
  if (transfer_handler->write) {
    devfs_async_t *async = transfer_handler->write;
    transfer_handler->write = NULL;
    if (nbyte == 0) {
      async->result = async->nbyte;
    } else {
      async->result = nbyte;
    }
    return devfs_execute_event_handler(&async->handler, o_flags, data);
  }
  return 0;
}
