/* Copyright 2011-2021 Tyler Gilbert;
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

#ifndef DEVICE_DEVICE_FIFO_H_
#define DEVICE_DEVICE_FIFO_H_

#include "fifo.h"
#include "sos/dev/device_fifo.h"
#include "sos/fs/devfs.h"

typedef struct {
  fifo_state_t fifo;
  devfs_async_t async;
  u32 count;
  s32 error;
} device_fifo_state_t;

typedef struct {
  const devfs_device_t *device;
  fifo_config_t fifo;
  int location;
  void *read_buffer;
  u32 read_buffer_size;
} device_fifo_config_t;

int device_fifo_open(const devfs_handle_t *handle);
int device_fifo_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int device_fifo_read(const devfs_handle_t *handle, devfs_async_t *async);
int device_fifo_write(const devfs_handle_t *handle, devfs_async_t *async);
int device_fifo_close(const devfs_handle_t *handle);

#define DEVICE_FIFO_DECLARE_CONFIG_STATE_RX_ONLY(                                        \
  name, frame_size_value, count_value, device_value)                                     \
  char name##_rx_buffer[count_value * frame_size_value];                                 \
  device_fifo_state_t name##_state MCU_SYS_MEM;                                          \
  device_fifo_config_t name##_config = {                                                 \
    .device = device_value,                                                              \
    .tx = {.count = 0, .frame_size = 0, .buffer = 0},                                    \
    .rx = {                                                                              \
      .count = count_value,                                                              \
      .frame_size = frame_size_value,                                                    \
      .buffer = name##_rx_buffer}}

#define DEVICE_FIFO_DECLARE_CONFIG_STATE_TX_ONLY(                                        \
  name, frame_size_value, count_value, device_value)                                     \
  char name##_tx_buffer[count_value * frame_size_value];                                 \
  device_fifo_state_t name##_state MCU_SYS_MEM;                                          \
  device_fifo_config_t name##_config = {                                                 \
    .device = device_value,                                                              \
    .rx = {.count = 0, .frame_size = 0, .buffer = 0},                                    \
    .tx = {                                                                              \
      .count = count_value,                                                              \
      .frame_size = frame_size_value,                                                    \
      .buffer = name##_tx_buffer}}

#define DEVICE_FIFO_DECLARE_CONFIG_STATE(                                                \
  name, frame_size_value, count_value, device_value)                                     \
  char name##_rx_buffer[count_value * frame_size_value];                                 \
  char name##_tx_buffer[count_value * frame_size_value];                                 \
  device_fifo_state_t name##_state MCU_SYS_MEM;                                          \
  device_fifo_config_t name##_config = {                                                 \
    .device = device_value,                                                              \
    .rx =                                                                                \
      {.count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer} \
        .tx = {                                                                          \
        .count = count_value,                                                            \
        .frame_size = frame_size_value,                                                  \
        .buffer = name##_tx_buffer}}

#endif /* DEVICE_DEVICE_FIFO_H_ */
