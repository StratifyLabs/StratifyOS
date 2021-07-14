// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_FFIFO_H_
#define DEVICE_FFIFO_H_

#include "device/fifo.h"
#include "sos/dev/ffifo.h"
#include "sos/fs/devfs.h"

typedef struct {
  volatile fifo_atomic_position_t atomic_position;
  devfs_transfer_handler_t transfer_handler;
  volatile u32 o_flags;
} ffifo_state_t;

typedef struct {
  u16 frame_count /*! The number of frames in the buffer */;
  u16 frame_size /*! The size of each frame */;
  char *buffer /*! A pointer to the buffer */;
} ffifo_config_t;

int ffifo_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int ffifo_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_EXEC_CODE;
int ffifo_read(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_EXEC_CODE;
int ffifo_write(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_EXEC_CODE;
int ffifo_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

int ffifo_open_local(const ffifo_config_t *config, ffifo_state_t *state)
  MCU_ROOT_EXEC_CODE;
int ffifo_close_local(const ffifo_config_t *config, ffifo_state_t *state)
  MCU_ROOT_EXEC_CODE;
int ffifo_ioctl_local(
  const ffifo_config_t *config,
  ffifo_state_t *state,
  int request,
  void *ctl) MCU_ROOT_EXEC_CODE;
int ffifo_write_local(
  const ffifo_config_t *config,
  ffifo_state_t *state,
  devfs_async_t *async,
  int allow_callback) MCU_ROOT_EXEC_CODE;
int ffifo_read_local(
  const ffifo_config_t *config,
  ffifo_state_t *state,
  devfs_async_t *async,
  int allow_callback) MCU_ROOT_EXEC_CODE;

// returns a pointer to the next place to write
void *
ffifo_get_head(const ffifo_config_t *config, ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
void *
ffifo_get_tail(const ffifo_config_t *config, ffifo_state_t *state) MCU_ROOT_EXEC_CODE;

int ffifo_read_buffer(
  const ffifo_config_t *config,
  ffifo_state_t *state,
  char *buf,
  int len) MCU_ROOT_EXEC_CODE;
int ffifo_write_buffer(
  const ffifo_config_t *config,
  ffifo_state_t *state,
  const char *buf,
  int len) MCU_ROOT_EXEC_CODE;

char *ffifo_get_frame(const ffifo_config_t *config, u16 frame) MCU_ROOT_EXEC_CODE;

// helper functions for implementing FIFOs
void ffifo_flush(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
int ffifo_getinfo(ffifo_info_t *info, const ffifo_config_t *config, ffifo_state_t *state)
  MCU_ROOT_EXEC_CODE;

void ffifo_inc_head(ffifo_state_t *state, u16 count) MCU_ROOT_EXEC_CODE;
void ffifo_inc_tail(ffifo_state_t *state, u16 count) MCU_ROOT_EXEC_CODE;
int ffifo_is_write_ok(ffifo_state_t *state, u16 count, int writeblock) MCU_ROOT_EXEC_CODE;

int ffifo_is_writeblock(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
void ffifo_set_writeblock(ffifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int ffifo_is_notify_read(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
void ffifo_set_notify_read(ffifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int ffifo_is_notify_write(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
void ffifo_set_notify_write(ffifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

int ffifo_is_overflow(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
void ffifo_set_overflow(ffifo_state_t *state, int value) MCU_ROOT_EXEC_CODE;

void ffifo_data_transmitted(const ffifo_config_t *cfgp, ffifo_state_t *state)
  MCU_ROOT_EXEC_CODE;
void ffifo_data_received(const ffifo_config_t *cfgp, ffifo_state_t *state)
  MCU_ROOT_EXEC_CODE;

void ffifo_cancel_async_read(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;
void ffifo_cancel_async_write(ffifo_state_t *state) MCU_ROOT_EXEC_CODE;

#define FFIFO_DEFINE_CONFIG(ffifo_frame_count, ffifo_frame_size, ffifo_buffer)           \
  .frame_count = ffifo_frame_count, .frame_size = ffifo_frame_size, .buffer = ffifo_buffer

#define FFIFO_DECLARE_CONFIG_STATE(ffifo_name, ffifo_frame_count, ffifo_frame_size)      \
  ffifo_state_t ffifo_name##_state MCU_SYS_MEM;                                          \
  char ffifo_name##_buffer[ffifo_frame_size * ffifo_frame_count];                        \
  const ffifo_config_t ffifo_name##_config = {                                           \
    FFIFO_DEFINE_CONFIG(ffifo_frame_count, ffifo_frame_size, ffifo_name##_buffer)}

#endif /* DEVICE_FFIFO_H_ */
