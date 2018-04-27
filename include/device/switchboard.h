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

#ifndef DEVICE_SWITCHBOARD_H_
#define DEVICE_SWITCHBOARD_H_

#include "sos/dev/switchboard.h"

typedef struct {
    const devfs_device_t * device;
    devfs_async_t async;
    u32 bytes_transferred;
} switchboard_state_terminal_t;

typedef struct {
    u32 o_flags;
    switchboard_state_terminal_t input;
    switchboard_state_terminal_t output;
    s32 nbyte; //total number of bytes -- set to 0 for persistent connections
    void * buffer[2]; //circular buffer pointers
    u16 bytes_in_buffer[2];
    u16 transaction_limit;
    u16 packet_size;
} switchboard_state_t;

typedef struct {
    const devfs_device_t * devfs_list; //pointer to the list of devices that contains the switchboard
    u16 connection_count; //max number of connections allowed
    u16 connection_buffer_size; //actual bytes available per transaction
    u16 transaction_limit; //max 65535 means users can't make this so high it triggers the WDT
    u16 resd;
    void * buffer; //array of buffers (connection_count * 2 * connection_buffer_size)
} switchboard_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int switchboard_open(const devfs_handle_t * handle);
int switchboard_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int switchboard_read(const devfs_handle_t * handle, devfs_async_t * rop);
int switchboard_write(const devfs_handle_t * handle, devfs_async_t * wop);
int switchboard_close(const devfs_handle_t * handle);

#define SWITCHBOARD_DECLARE_CONFIG_STATE(switchboard_name, devfs_list_value, connection_count_value, connection_buffer_size_value, \
    transaction_limit_value ) \
    char switchboard_name##_buffer[connection_count_value*connection_buffer_size_value*2]; \
    switchboard_state_t switchboard_name##_state[connection_count_value] MCU_SYS_MEM; \
    const switchboard_config_t switchboard_name##_config = { \
      .devfs_list = devfs_list_value, \
      .connection_count = connection_count_value, \
      .connection_buffer_size = connection_buffer_size_value, \
      .transaction_limit = transaction_limit_value, \
      .buffer = switchboard_name##_buffer \
    }


#ifdef __cplusplus
}
#endif


#endif /* DEVICE_SWITCHBOARD_H_ */

