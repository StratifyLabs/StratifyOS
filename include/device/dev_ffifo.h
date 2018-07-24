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

/*! \addtogroup DEV_FFIFO
 * @{
 * \ingroup IFACE_DEV
 *
 */

/*! \file  */

#ifndef DEVICE_DEV_FFIFO_H_
#define DEVICE_DEV_FFIFO_H_

#include "sos/fs/devfs.h"
#include "sos/dev/dev_ffifo.h"
#include "ffifo.h"


typedef struct {
    ffifo_state_t ffifo;
    devfs_async_t async;
    u32 count;
    s32 error;
} dev_ffifo_channel_state_t;

typedef struct {
    dev_ffifo_channel_state_t rx;
    dev_ffifo_channel_state_t tx;
} dev_ffifo_state_t;

typedef struct {
    ffifo_config_t tx;
    ffifo_config_t rx;
    const devfs_device_t * device;
} dev_ffifo_config_t;


int dev_ffifo_open(const devfs_handle_t * handle);
int dev_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int dev_ffifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int dev_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int dev_ffifo_close(const devfs_handle_t * handle);

#define DEV_FFIFO_DECLARE_CONFIG_STATE_RX_ONLY(name, \
    frame_size_value, \
    count_value, \
    device_value) \
    char name##_rx_buffer[count_value*frame_size_value]; \
    dev_ffifo_state_t name##_state MCU_SYS_MEM; \
    dev_ffifo_config_t name##_config = { \
    .device = device_value, \
    .tx = { .count = 0, .frame_size = 0, .buffer = 0 }, \
    .rx = { .count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer } \
    }

#define DEV_FFIFO_DECLARE_CONFIG_STATE_TX_ONLY(name, \
    frame_size_value, \
    count_value, \
    device_value) \
    char name##_tx_buffer[count_value*frame_size_value]; \
    dev_ffifo_state_t name##_state MCU_SYS_MEM; \
    dev_ffifo_config_t name##_config = { \
    .device = device_value, \
    .rx = { .count = 0, .frame_size = 0, .buffer = 0 }, \
    .tx = { .count = count_value, .frame_size = frame_size_value, .buffer = name##_tx_buffer } \
    }

#define DEV_FFIFO_DECLARE_CONFIG_STATE(name, \
    frame_size_value, \
    count_value, \
    device_value) \
    char name##_rx_buffer[count_value*frame_size_value]; \
    char name##_tx_buffer[count_value*frame_size_value]; \
    dev_ffifo_state_t name##_state MCU_SYS_MEM; \
    dev_ffifo_config_t name##_config = { \
    .device = device_value, \
    .rx = { .count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer } \
    .tx = { .count = count_value, .frame_size = frame_size_value, .buffer = name##_tx_buffer } \
    }



#endif /* DEVICE_DEV_FFIFO_H_ */


/*! @} */
