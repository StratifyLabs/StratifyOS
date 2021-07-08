// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_FIFO_H_
#define DEVICE_FIFO_H_

#include "sos/dev/fifo.h"
#include "sos/fs/devfs.h"

typedef struct MCU_PACK {
  u16 head;
  u16 tail;
} fifo_position_t;

typedef union {
  fifo_position_t access;
  u32 atomic_access; // read head and tail in one operation
} fifo_atomic_position_t;

typedef struct {
  volatile fifo_atomic_position_t atomic_position; // 4 bytes
  devfs_transfer_handler_t transfer_handler;       // 8 bytes
  volatile u32 o_flags;                            // 4 bytes
} fifo_state_t;

typedef struct MCU_PACK {
  u32 size;
  char *buffer;
} fifo_config_t;

int fifo_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int fifo_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_EXEC_CODE;
int fifo_read(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_EXEC_CODE;
int fifo_write(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_EXEC_CODE;
int fifo_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

int fifo_open_local(const fifo_config_t *config, fifo_state_t *state) MCU_ROOT_EXEC_CODE;
int fifo_close_local(const fifo_config_t *config, fifo_state_t *state) MCU_ROOT_EXEC_CODE;
int fifo_write_local(
  const fifo_config_t *config,
  fifo_state_t *state,
  devfs_async_t *async,
  int allow_callback) MCU_ROOT_EXEC_CODE;
int fifo_read_local(
  const fifo_config_t *config,
  fifo_state_t *state,
  devfs_async_t *async,
  int allow_callback) MCU_ROOT_EXEC_CODE;
int fifo_ioctl_local(
  const fifo_config_t *config,
  fifo_state_t *state,
  int request,
  void *ctl) MCU_ROOT_EXEC_CODE;

// helper functions for implementing FIFOs
void fifo_flush(fifo_state_t *state) MCU_ROOT_EXEC_CODE;
void fifo_getinfo(fifo_info_t *info, const fifo_config_t *cfgp, fifo_state_t *state)
  MCU_ROOT_EXEC_CODE;

void fifo_inc_head(fifo_state_t *state, int size) MCU_ROOT_EXEC_CODE;
void fifo_inc_tail(fifo_state_t *state, int size) MCU_ROOT_EXEC_CODE;
int fifo_is_write_ok(fifo_state_t *state, u16 size, int writeblock) MCU_ROOT_EXEC_CODE;

int fifo_is_writeblock(fifo_state_t *state) MCU_ROOT_EXEC_CODE;
void fifo_set_writeblock(fifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int fifo_is_notify_read(fifo_state_t *state) MCU_ROOT_EXEC_CODE;
void fifo_set_notify_read(fifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int fifo_is_notify_write(fifo_state_t *state) MCU_ROOT_EXEC_CODE;
void fifo_set_notify_write(fifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int fifo_is_overflow(fifo_state_t *state) MCU_ROOT_EXEC_CODE;
void fifo_set_overflow(fifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int fifo_read_buffer(
  const fifo_config_t *cfgp,
  fifo_state_t *state,
  char *buf,
  int nbyte);
int fifo_write_buffer(
  const fifo_config_t *cfgp,
  fifo_state_t *state,
  const char *buf,
  int nbyte,
  int non_blocking) MCU_ROOT_EXEC_CODE;

int fifo_data_transmitted(const fifo_config_t *cfgp, fifo_state_t *state)
  MCU_ROOT_EXEC_CODE;
void fifo_data_received(const fifo_config_t *cfgp, fifo_state_t *state)
  MCU_ROOT_EXEC_CODE;

void fifo_cancel_async_read(fifo_state_t *state) MCU_ROOT_EXEC_CODE;
void fifo_cancel_async_write(fifo_state_t *state) MCU_ROOT_EXEC_CODE;

#define FIFO_DEFINE_CONFIG(fifo_size, fifo_buffer)                                       \
  .size = fifo_size, .buffer = fifo_buffer

#define FIFO_DECLARE_CONFIG_STATE(fifo_name, fifo_buffer_size)                           \
  fifo_state_t fifo_name##_state MCU_SYS_MEM;                                            \
  char fifo_name##_buffer[fifo_buffer_size] MCU_SYS_MEM;                                 \
  const fifo_config_t fifo_name##_config = {                                             \
    FIFO_DEFINE_CONFIG(fifo_buffer_size, fifo_name##_buffer)}

#endif /* DEVICE_FIFO_H_ */
