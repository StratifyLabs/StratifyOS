// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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
  devfs_device_t device;
  fifo_config_t fifo;
  int read_location;
  int write_location;
  void *read_buffer;
  u32 read_buffer_size;
} device_fifo_config_t;

int device_fifo_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int device_fifo_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int device_fifo_read(const devfs_handle_t *handle, devfs_async_t *async)
  MCU_ROOT_EXEC_CODE;
int device_fifo_write(const devfs_handle_t *handle, devfs_async_t *async)
  MCU_ROOT_EXEC_CODE;
int device_fifo_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

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
