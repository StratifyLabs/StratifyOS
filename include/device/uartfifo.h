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

#ifndef DEV_UARTFIFO_H_
#define DEV_UARTFIFO_H_

#include "sos/dev/uartfifo.h"
#include "mcu/uart.h"
#include "fifo.h"


/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
	fifo_state_t fifo;
	devfs_async_t async_read;
} uartfifo_state_t;



/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
	uart_config_t uart;
	fifo_config_t fifo;
	u16 read_buffer_size;
	char * read_buffer;
} uartfifo_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int uartfifo_open(const devfs_handle_t * handle);
int uartfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int uartfifo_read(const devfs_handle_t * handle, devfs_async_t * rop);
int uartfifo_write(const devfs_handle_t * handle, devfs_async_t * wop);
int uartfifo_close(const devfs_handle_t * handle);

#define UARTFIFO_DECLARE_CONFIG_STATE(uart_fifo_name,\
	uart_fifo_buffer_size, \
	uart_fifo_read_buffer_size, \
	uart_attr_flags, \
	uart_attr_width, \
	uart_attr_freq, \
	uart_attr_tx_port, uart_attr_tx_pin, \
	uart_attr_rx_port, uart_attr_rx_pin, \
	uart_attr_cts_port, uart_attr_cts_pin, \
	uart_attr_rts_port, uart_attr_rts_pin ) \
	uartfifo_state_t uart_fifo_name##_state MCU_SYS_MEM; \
	char uart_fifo_name##_buffer[uart_fifo_buffer_size] MCU_SYS_MEM; \
	char uart_fifo_name##_read_buffer[uart_fifo_read_buffer_size] MCU_SYS_MEM; \
	const uartfifo_config_t uart_fifo_name##_config = { \
	.uart = { .attr = { UART_DEFINE_ATTR( uart_attr_flags, \
	uart_attr_width, \
	uart_attr_freq, \
	uart_attr_tx_port, uart_attr_tx_pin, \
	uart_attr_rx_port, uart_attr_rx_pin, \
	uart_attr_cts_port, uart_attr_cts_pin, \
	uart_attr_rts_port, uart_attr_rts_pin ) }}, \
	.fifo = { .size = uart_fifo_buffer_size, .buffer = uart_fifo_name##_buffer }, \
	.read_buffer = uart_fifo_name##_read_buffer, \
	.read_buffer_size = uart_fifo_read_buffer_size }


#ifdef __cplusplus
}
#endif


#endif /* DEV_UARTFIFO_H_ */

