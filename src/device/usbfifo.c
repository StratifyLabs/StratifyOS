// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "device/usbfifo.h"
#include "cortexm/task.h"
#include "mcu/usb.h"
#include "sos/debug.h"
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>

static int cancel_read_action(const devfs_handle_t *handle) {
  mcu_action_t action;
  const usbfifo_config_t *config = handle->config;

  action.handler.callback = 0;
  action.handler.context = 0;
  action.o_events = MCU_EVENT_FLAG_DATA_READY;
  action.channel = config->endpoint;
  action.prio = 0;
  return config->device.driver.ioctl(&config->device.handle, I_USB_SETACTION, &action);
}

static int data_received(void *context, const mcu_event_t *data) {
  int i;
  const devfs_handle_t *handle;
  const usbfifo_config_t *config;
  usbfifo_state_t *state;
  handle = context;
  config = handle->config;
  state = handle->state;
  int result;
  int size = config->fifo.size;

  result = state->async_read.result;

  do {

    if (result > 0) {

      // write the new bytes to the buffer
      for (i = 0; i < result; i++) {
        config->fifo.buffer[state->fifo.atomic_position.access.head] =
          config->read_buffer[i];
        fifo_inc_head(&(state->fifo), size);
      }

      // see if any functions are blocked waiting for data to arrive
      fifo_data_received(&(config->fifo), &(state->fifo));
    }

    // if this returns > 0 then data is ready right now
    result = config->device.driver.read(&config->device.handle, &state->async_read);
    if (result < 0) {
      // EAGAIN can happen if too much data arrives at one time
      if (SYSFS_GET_RETURN_ERRNO(result) == EAGAIN) {
        // cortexm_delay_ms(1);
        result = config->device.driver.read(&config->device.handle, &state->async_read);
      }

      if (result < 0) {
        // fire an error -- set this as an error condition
        sos_debug_log_error(
          SOS_DEBUG_DEVICE, "failed to read USB (%d, %d)", SYSFS_GET_RETURN(result),
          SYSFS_GET_RETURN_ERRNO(result));
        return 0;
      }
    }

  } while (result > 0);

  return 0; // done
}

int usbfifo_open(const devfs_handle_t *handle) {
  const usbfifo_config_t *config = handle->config;
  usbfifo_state_t *state = handle->state;

  if (fifo_open_local(&config->fifo, &state->fifo) < 0) {
    return -1;
  }

  return config->device.driver.open(handle);
}

int usbfifo_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  fifo_info_t *info = ctl;
  mcu_action_t *action = ctl;
  const usbfifo_config_t *config = handle->config;
  usbfifo_state_t *state = handle->state;
  int result;
  int count;
  switch (request) {
  case I_FIFO_GETINFO:
    fifo_getinfo(info, &(config->fifo), &(state->fifo));
    break;
  case I_USB_SETACTION:
  case I_MCU_SETACTION:
    if (action->handler.callback == 0) {
      fifo_cancel_async_read(&(state->fifo));
    } else {
      return config->device.driver.ioctl(&config->device.handle, request, ctl);
    }
    return 0;
  case I_FIFO_FLUSH:
    fifo_flush(&(state->fifo));
    devfs_execute_read_handler(
      &state->fifo.transfer_handler, 0, -1, MCU_EVENT_FLAG_CANCELED);
    break;
  case I_USB_SETATTR: {
    usb_attr_t *attr = ctl;
    if (attr->o_flags & USB_FLAG_SET_DEVICE) {
      fifo_flush(&(state->fifo));
      fifo_cancel_async_read(&(state->fifo));
    }
  }

    // setup the device to write to the fifo when data arrives
    result = config->device.driver.ioctl(&config->device.handle, I_USB_SETATTR, ctl);
    if (result < 0) {
      return result;
    }
    /* no break */
  case I_FIFO_INIT:
    fifo_flush(&(state->fifo));
    state->async_read.tid = task_get_current();
    state->async_read.flags = 0;
    state->async_read.handler.callback = data_received;
    state->async_read.handler.context = (void *)handle;
    state->async_read.loc = config->endpoint;
    state->async_read.buf = config->read_buffer;
    state->async_read.nbyte = config->endpoint_size;

    count = 0;
    do {
      // flush the USB and get an async call
      result = config->device.driver.read(&config->device.handle, &state->async_read);
      count++;
    } while ((result > 0) && (count < 10));
    if (result < 0) {
      // sos_debug_printf("failed to init USB fifo (%d)\n", SYSFS_GET_RETURN(result));
      return SYSFS_SET_RETURN(EIO);
    }
    if (count == 0) {
      // sos_debug_printf("failed to init USB fifo -- count\n");
      return SYSFS_SET_RETURN(EIO);
    }

    break;
  case I_FIFO_EXIT:
    // clear the callback for the device
    result = cancel_read_action(handle);
    if (result < 0) {
      return SYSFS_SET_RETURN(EIO);
    }
    return config->device.driver.close(handle);
  default:
    return config->device.driver.ioctl(&config->device.handle, request, ctl);
  }
  return 0;
}

int usbfifo_read(const devfs_handle_t *handle, devfs_async_t *async) {
  const usbfifo_config_t *config = handle->config;
  usbfifo_state_t *state = handle->state;
  return fifo_read_local(&(config->fifo), &(state->fifo), async, 1);
}

int usbfifo_write(const devfs_handle_t *handle, devfs_async_t *async) {
  const usbfifo_config_t *config = handle->config;
  async->loc = 0x80 | config->endpoint;
  // Writing to the USB FIFO is not buffered, it just writes the USB HW directly
  if (config->device.driver.ioctl(&config->device.handle, I_USB_ISCONNECTED, NULL) == 0) {
    return SYSFS_SET_RETURN(ENODEV);
  }

  return config->device.driver.write(&config->device.handle, async);
}

int usbfifo_close(const devfs_handle_t *handle) {
  const usbfifo_config_t *config = handle->config;
  // use I_FIFO_EXIT to close the USB
  return config->device.driver.close(&config->device.handle);
}
