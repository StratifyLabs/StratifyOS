// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_STREAM_FFIFO_H_
#define DEVICE_STREAM_FFIFO_H_

#include "ffifo.h"
#include "sos/dev/stream_ffifo.h"
#include "sos/fs/devfs.h"

typedef struct {
  ffifo_state_t ffifo;
  devfs_async_t async;
  u32 access_count;
  s32 error;
} stream_ffifo_channel_state_t;

typedef struct {
  stream_ffifo_channel_state_t rx;
  stream_ffifo_channel_state_t tx;
  u32 o_flags;
} stream_ffifo_state_t;

typedef struct {
  ffifo_config_t tx;
  ffifo_config_t rx;
  devfs_device_t device;
  u32 rx_loc;
  u32 tx_loc;
} stream_ffifo_config_t;

int stream_ffifo_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int stream_ffifo_ioctl(const devfs_handle_t *handle, int request, void *ctl)
  MCU_ROOT_EXEC_CODE;
int stream_ffifo_read(const devfs_handle_t *handle, devfs_async_t *async)
  MCU_ROOT_EXEC_CODE;
int stream_ffifo_write(const devfs_handle_t *handle, devfs_async_t *async)
  MCU_ROOT_EXEC_CODE;
int stream_ffifo_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

#define STREAM_FFIFO_DECLARE_CONFIG_STATE_RX_ONLY(                                       \
  name, frame_size_value, count_value, device_value, loc_value)                          \
  char name##_rx_buffer[count_value * frame_size_value] MCU_ALIGN(32);                   \
  stream_ffifo_state_t name##_state MCU_SYS_MEM;                                         \
  const stream_ffifo_config_t name##_config = {                                          \
    .device = device_value,                                                              \
    .tx_loc = 0,                                                                         \
    .rx_loc = loc_value,                                                                 \
    .tx = {.frame_count = 0, .frame_size = 0, .buffer = 0},                              \
    .rx = {                                                                              \
      .frame_count = count_value,                                                        \
      .frame_size = frame_size_value,                                                    \
      .buffer = name##_rx_buffer}}

#define STREAM_FFIFO_DECLARE_CONFIG_STATE_TX_ONLY(                                       \
  name, frame_size_value, count_value, device_value, loc_value)                          \
  char name##_tx_buffer[count_value * frame_size_value] MCU_ALIGN(32);                   \
  stream_ffifo_state_t name##_state MCU_SYS_MEM;                                         \
  const stream_ffifo_config_t name##_config = {                                          \
    .device = device_value,                                                              \
    .rx_loc = 0,                                                                         \
    .tx_loc = loc_value,                                                                 \
    .rx = {.frame_count = 0, .frame_size = 0, .buffer = 0},                              \
    .tx = {                                                                              \
      .frame_count = count_value,                                                        \
      .frame_size = frame_size_value,                                                    \
      .buffer = name##_tx_buffer}}

#define STREAM_FFIFO_DECLARE_CONFIG_STATE(                                               \
  name, frame_size_value, count_value, device_value, tx_loc_value, rx_loc_value)         \
  char name##_rx_buffer[count_value * frame_size_value] MCU_ALIGN(32);                   \
  char name##_tx_buffer[count_value * frame_size_value] MCU_ALIGN(32);                   \
  stream_ffifo_state_t name##_state MCU_SYS_MEM;                                         \
  stream_ffifo_config_t name##_config = {                                                \
    .device = device_value,                                                              \
    .rx_loc = rx_loc_value,                                                              \
    .tx_loc = tx_loc_value,                                                              \
    .rx =                                                                                \
      {.frame_count = count_value,                                                       \
       .frame_size = frame_size_value,                                                   \
       .buffer = name##_rx_buffer},                                                      \
    .tx = {                                                                              \
      .frame_count = count_value,                                                        \
      .frame_size = frame_size_value,                                                    \
      .buffer = name##_tx_buffer}}

#endif /* DEVICE_STREAM_FFIFO_H_ */
