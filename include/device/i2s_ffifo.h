// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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
    u32 access_count;
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
	.tx = { .frame_count = 0, .frame_size = 0, .buffer = 0 }, \
	.rx = { .frame_count = count_value, .frame_size = frame_size_value, .buffer = name##_rx_buffer } \
    }



#endif /* DEVICE_I2S_FFIFO_H_ */


/*! @} */
