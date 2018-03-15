/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup UART_DEV UART
 * @{
 *
 * \ingroup DEV
 *
 */

#ifndef _MCU_UART_H_
#define _MCU_UART_H_

#include "sos/dev/uart.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
    u32 value;
} uart_event_t;

typedef struct MCU_PACK {
    uart_attr_t attr; //default attributes
} uart_config_t;


int mcu_uart_open(const devfs_handle_t * cfg) MCU_ROOT_CODE;
int mcu_uart_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_uart_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_uart_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_ROOT_CODE;
int mcu_uart_close(const devfs_handle_t * cfg) MCU_ROOT_CODE;

int mcu_uart_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_uart_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_uart_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

int mcu_uart_get(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_uart_put(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_uart_flush(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

#define UART_DEFINE_ATTR(uart_attr_flags, \
    uart_attr_width, \
    uart_attr_freq, \
    uart_attr_tx_port, uart_attr_tx_pin, \
    uart_attr_rx_port, uart_attr_rx_pin, \
    uart_attr_rts_port, uart_attr_rts_pin, \
    uart_attr_cts_port, uart_attr_cts_pin) \
    .o_flags = uart_attr_flags, .width = uart_attr_width, .freq = uart_attr_freq, \
    .pin_assignment.tx = {uart_attr_tx_port, uart_attr_tx_pin}, .pin_assignment.rx = {uart_attr_rx_port, uart_attr_rx_pin}, \
    .pin_assignment.rts = {uart_attr_rts_port, uart_attr_rts_pin}, .pin_assignment.cts = {uart_attr_cts_port, uart_attr_cts_pin}

#define UART_DECLARE_CONFIG(uart_name, \
    uart_attr_flags, \
    uart_attr_width, \
    uart_attr_freq, \
    uart_attr_tx_port, uart_attr_tx_pin, \
    uart_attr_rx_port, uart_attr_rx_pin, \
    uart_attr_rts_port, uart_attr_rts_pin, \
    uart_attr_cts_port, uart_attr_cts_pin) \
    uart_config_t uart_name##config = { .attr = \
{ UART_DEFINE_ATTR( uart_attr_flags, \
    uart_attr_width, \
    uart_attr_freq, \
    uart_attr_tx_port, uart_attr_tx_pin, \
    uart_attr_rx_port, uart_attr_rx_pin, \
    uart_attr_rts_port, uart_attr_rts_pin, \
    uart_attr_cts_port, uart_attr_cts_pin ) } }

#ifdef __cplusplus
}
#endif


#endif // _MCU_UART_H_

/*! @} */

