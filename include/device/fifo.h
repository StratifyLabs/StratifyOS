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
    u32 atomic_access; //read head and tail in one operation
} fifo_atomic_position_t;

typedef struct MCU_PACK {
    volatile fifo_atomic_position_t atomic_position; //4 bytes
    devfs_transfer_handler_t transfer_handler; //8 bytes
    volatile u32 o_flags; //4 bytes
} fifo_state_t;

/*! \brief FIFO Configuration
 * \details This structure defines the static FIFO configuration.
 *
 */
typedef struct MCU_PACK {
	u32 size /*! \brief The size of the buffer (only size-1 is usable) */;
	char * buffer /*! \brief A pointer to the buffer */;
} fifo_config_t;

int fifo_open(const devfs_handle_t * handle);
int fifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int fifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int fifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int fifo_close(const devfs_handle_t * handle);

int fifo_open_local(const fifo_config_t * config, fifo_state_t * state);
int fifo_close_local(const fifo_config_t * config, fifo_state_t * state);
int fifo_write_local(const fifo_config_t * config, fifo_state_t * state, devfs_async_t * async, int allow_callback);
int fifo_read_local(const fifo_config_t * config, fifo_state_t * state, devfs_async_t * async, int allow_callback);
int fifo_ioctl_local(const fifo_config_t * config, fifo_state_t * state, int request, void * ctl);


//helper functions for implementing FIFOs
void fifo_flush(fifo_state_t * state);
void fifo_getinfo(fifo_info_t * info, const fifo_config_t * cfgp, fifo_state_t * state);

void fifo_inc_head(fifo_state_t * state, int size);
void fifo_inc_tail(fifo_state_t * state, int size);
int fifo_is_write_ok(fifo_state_t * state, u16 size, int writeblock);

int fifo_is_writeblock(fifo_state_t * state);
void fifo_set_writeblock(fifo_state_t * state, int value);

int fifo_is_notify_read(fifo_state_t * state);
void fifo_set_notify_read(fifo_state_t * state, int value);

int fifo_is_notify_write(fifo_state_t * state);
void fifo_set_notify_write(fifo_state_t * state, int value);

int fifo_is_overflow(fifo_state_t * state);
void fifo_set_overflow(fifo_state_t * state, int value);

int fifo_read_buffer(const fifo_config_t * cfgp, fifo_state_t * state, char * buf, int nbyte);
int fifo_write_buffer(const fifo_config_t * cfgp, fifo_state_t * state, const char * buf, int nbyte, int non_blocking);

int fifo_data_transmitted(const fifo_config_t * cfgp, fifo_state_t * state);
void fifo_data_received(const fifo_config_t * cfgp, fifo_state_t * state);

void fifo_cancel_async_read(fifo_state_t * state);
void fifo_cancel_async_write(fifo_state_t * state);


#define FIFO_DEFINE_CONFIG(fifo_size, fifo_buffer) .size = fifo_size, .buffer = fifo_buffer

#define FIFO_DECLARE_CONFIG_STATE(fifo_name,\
    fifo_buffer_size ) \
    fifo_state_t fifo_name##_state MCU_SYS_MEM; \
    char fifo_name##_buffer[fifo_buffer_size] MCU_SYS_MEM; \
    const fifo_config_t fifo_name##_config = { FIFO_DEFINE_CONFIG(fifo_buffer_size, fifo_name##_buffer) }

#endif /* DEVICE_FIFO_H_ */

