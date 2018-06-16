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

/*! \addtogroup USB_FIFO
 * @{
 * \ingroup IFACE_DEV
 *
 */

/*! \file  */

#ifndef DEVICE_I2S_FFIFO_H_
#define DEVICE_I2S_FFIFO_H_

#include "sos/dev/i2s_ffifo.h"
#include "mcu/i2s.h"
#include "ffifo.h"





typedef struct {
    ffifo_state_t ffifo;
    devfs_async_t i2s_async;
    u32 count;
    s32 error;
} i2s_ffifo_channel_state_t;

typedef struct {
    i2s_ffifo_channel_state_t rx;
    i2s_ffifo_channel_state_t tx;
} i2s_ffifo_state_t;

typedef struct {
    i2s_config_t i2s;
    ffifo_config_t tx;
    ffifo_config_t rx;
} i2s_ffifo_config_t;


int i2s_ffifo_open(const devfs_handle_t * handle);
int i2s_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int i2s_ffifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int i2s_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int i2s_ffifo_close(const devfs_handle_t * handle);

int i2s_spi_ffifo_open(const devfs_handle_t * handle);
int i2s_spi_ffifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int i2s_spi_ffifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int i2s_spi_ffifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int i2s_spi_ffifo_close(const devfs_handle_t * handle);

#define I2S_FFIFO_DECLARE_CONFIG_STATE_RX_ONLY(name, \
    frame_size_value, \
    count_value, \
    attr_flags, \
    attr_freq, \
    attr_mck_mult, \
    attr_ws_port, attr_ws_pin, \
    attr_sck_port, attr_sck_pin, \
    attr_sdin_port, attr_sdin_pin, \
    attr_mck_port, attr_mck_pin) \
    char name##_rx_buffer[count_value*frame_size_value]; \
    i2s_ffifo_state_t name##_state MCU_SYS_MEM; \
    i2s_ffifo_config_t name##_config = { \
    .i2s = { .attr = { I2S_DEFINE_ATTR(attr_flags, \
    attr_freq, \
    attr_mck_mult, \
    attr_ws_port, attr_ws_pin, \
    attr_sck_port, attr_sck_pin, \
    0xff, 0xff, \
    attr_sdin_port, attr_sdin_pin, \
    attr_mck_port, attr_mck_pin) } }, \
    .tx = { .count = 0, .frame_size = 0, .buffer = 0 }, \
    .rx = { .count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer } \
    }



#endif /* DEVICE_I2S_FFIFO_H_ */


/*! @} */
