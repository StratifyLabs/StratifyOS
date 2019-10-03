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


#ifndef DEVICE_STREAM_FFIFO_H_
#define DEVICE_STREAM_FFIFO_H_

#include "sos/fs/devfs.h"
#include "sos/dev/stream_ffifo.h"
#include "ffifo.h"


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
    const devfs_device_t * device;
    u32 rx_loc;
    u32 tx_loc;
} stream_ffifo_config_t;


int stream_ffifo_open(const devfs_handle_t * handle);
int stream_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int stream_ffifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int stream_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int stream_ffifo_close(const devfs_handle_t * handle);

#define STREAM_FFIFO_DECLARE_CONFIG_STATE_RX_ONLY(name, \
    frame_size_value, \
    count_value, \
    device_value, \
    loc_value) \
    char name##_rx_buffer[count_value*frame_size_value] MCU_ALIGN(4); \
    stream_ffifo_state_t name##_state MCU_SYS_MEM; \
    stream_ffifo_config_t name##_config = { \
    .device = device_value, \
    .tx_loc = 0, \
    .rx_loc = loc_value, \
    .tx = { .frame_count = 0, .frame_size = 0, .buffer = 0 }, \
    .rx = { .frame_count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer } \
    }

#define STREAM_FFIFO_DECLARE_CONFIG_STATE_TX_ONLY(name, \
    frame_size_value, \
    count_value, \
    device_value, \
    loc_value) \
    char name##_tx_buffer[count_value*frame_size_value] MCU_ALIGN(4); \
    stream_ffifo_state_t name##_state MCU_SYS_MEM; \
    stream_ffifo_config_t name##_config = { \
    .device = device_value, \
    .rx_loc = 0, \
    .tx_loc = loc_value, \
    .rx = { .frame_count = 0, .frame_size = 0, .buffer = 0 }, \
    .tx = { .frame_count = count_value, .frame_size = frame_size_value, .buffer = name##_tx_buffer } \
    }

#define STREAM_FFIFO_DECLARE_CONFIG_STATE(name, \
    frame_size_value, \
    count_value, \
    device_value, \
    tx_loc_value, rx_loc_value ) \
    char name##_rx_buffer[count_value*frame_size_value] MCU_ALIGN(4); \
    char name##_tx_buffer[count_value*frame_size_value] MCU_ALIGN(4); \
    stream_ffifo_state_t name##_state MCU_SYS_MEM; \
    stream_ffifo_config_t name##_config = { \
    .device = device_value, \
    .rx_loc = rx_loc_value, \
    .tx_loc = tx_loc_value, \
	 .rx = { .frame_count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer }, \
    .tx = { .frame_count = count_value, .frame_size = frame_size_value, .buffer = name##_tx_buffer } \
    }



#endif /* DEVICE_STREAM_FFIFO_H_ */

