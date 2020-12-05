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

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "device/device_fifo.h"
#include "sos/debug.h"
#include "sos/events.h"

static int event_data_ready(void *context, const mcu_event_t *event);

static int cancel_read_action(const devfs_handle_t *handle) {
  mcu_action_t action;
  const device_fifo_config_t *config = handle->config;

  action.handler.callback = 0;
  action.handler.context = 0;
  action.o_events = MCU_EVENT_FLAG_DATA_READY;
  action.channel = config->read_location;
  action.prio = 0;
  return config->device.driver.ioctl(&config->device.handle, I_MCU_SETACTION, &action);
}

int event_data_ready(void *context, const mcu_event_t *event) {
  const devfs_handle_t *handle = context;
  const device_fifo_config_t *config = handle->config;
  device_fifo_state_t *state = handle->state;

  // data has been received from the device -- data needs to be
  // written to the FIFO and a new read initiated

  int result = state->async.result;
  u8 *source_buffer = config->read_buffer;
  u8 *dest_buffer = config->fifo.buffer;
  fifo_state_t *fifo_state = &state->fifo;

  do {

    if (result > 0) {
      for (int i = 0; i < result; i++) {
        dest_buffer[fifo_state->atomic_position.access.head] = source_buffer[i];
        fifo_inc_head(fifo_state, config->fifo.size);
      }

      // see if any functions are blocked waiting for data to arrive
      fifo_data_received(&(config->fifo), fifo_state);
    }

    // if this returns > 0 then data is ready right now
    result = config->device.driver.read(&config->device.handle, &state->async);
    if (result < 0) {
      // EAGAIN can happen if too much data arrives at one time
      if (SYSFS_GET_RETURN_ERRNO(result) == EAGAIN) {
        // cortexm_delay_ms(1);
        result = config->device.driver.read(handle, &state->async);
      }

      if (result < 0) {
        // fire an error -- set this as an error condition
        sos_debug_log_error(
          SOS_DEBUG_DEVICE, "failed to read device (%d, %d)", SYSFS_GET_RETURN(result),
          SYSFS_GET_RETURN_ERRNO(result));
        return 0;
      }
    }

  } while (result > 0);

  return 0;
}

int device_fifo_open(const devfs_handle_t *handle) {
  const device_fifo_config_t *config = handle->config;
  device_fifo_state_t *state = handle->state;
  int result;

  result = config->device.driver.open(&config->device.handle);
  if (result < 0) {
    return result;
  }

  result = fifo_open_local(&config->fifo, &state->fifo);
  if (result < 0) {
    return result;
  }

  return SYSFS_RETURN_SUCCESS;
}

int device_fifo_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  fifo_info_t *info = ctl;
  mcu_action_t *action = ctl;
  const device_fifo_config_t *config = handle->config;
  device_fifo_state_t *state = handle->state;

  int result;
  int count;
  switch (request) {
  case I_FIFO_GETINFO:
    fifo_getinfo(info, &(config->fifo), &(state->fifo));
    break;
  case I_MCU_SETACTION:
    if (action->handler.callback == 0) {
      fifo_cancel_async_read(&(state->fifo));
    }
    return 0;
  case I_FIFO_FLUSH:
    fifo_flush(&(state->fifo));
    devfs_execute_read_handler(
      &state->fifo.transfer_handler, 0, -1, MCU_EVENT_FLAG_CANCELED);
    break;
  case I_FIFO_INIT:
    fifo_cancel_async_read(&(state->fifo));

    sos_handle_event(SOS_EVENT_FIFO_INIT_REQUESTED, (void *)handle);

    fifo_flush(&(state->fifo));
    state->async.tid = task_get_current();
    state->async.flags = O_RDWR;
    state->async.handler.callback = event_data_ready;
    state->async.handler.context = (void *)handle;
    state->async.loc = config->read_location;
    state->async.buf = config->read_buffer;
    state->async.nbyte = config->read_buffer_size;

    count = 0;
    do {
      // flush the device
      result = config->device.driver.read(&config->device.handle, &state->async);
      count++;
    } while ((result > 0) && (count < 10));

    if (result < 0) {
      return SYSFS_SET_RETURN(EIO);
    }
    break;

  case I_FIFO_EXIT:
    // clear the callback for the device
    result = cancel_read_action(handle);
    if (result < 0) {
      return SYSFS_SET_RETURN(EIO);
    }
  }

  return config->device.driver.ioctl(&config->device.handle, request, ctl);
}

int device_fifo_read(const devfs_handle_t *handle, devfs_async_t *async) {
  const device_fifo_config_t *config = handle->config;
  device_fifo_state_t *state = handle->state;
  sos_handle_event(SOS_EVENT_FIFO_READ, (void *)handle);
  return fifo_read_local(&(config->fifo), &(state->fifo), async, 1);
}

int device_fifo_write(const devfs_handle_t *handle, devfs_async_t *async) {
  const device_fifo_config_t *config = handle->config;
  sos_handle_event(SOS_EVENT_FIFO_WRITE, (void *)handle);
  if (config->write_location > -1) {
    async->loc = config->write_location;
  }
  return config->device.driver.write(&config->device.handle, async);
}

int device_fifo_close(const devfs_handle_t *handle) {
  const device_fifo_config_t *config = handle->config;
  device_fifo_state_t *state = handle->state;
  fifo_close_local(&(config->fifo), &(state->fifo));
  return config->device.driver.close(&config->device.handle);
}
