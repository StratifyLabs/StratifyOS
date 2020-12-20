// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEV_UARTFIFO_H_
#define DEV_UARTFIFO_H_

#include "fifo.h"
#include "mcu/uart.h"
#include "sos/dev/uartfifo.h"

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
  char *read_buffer;
} uartfifo_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int uartfifo_open(const devfs_handle_t *handle);
int uartfifo_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int uartfifo_read(const devfs_handle_t *handle, devfs_async_t *rop);
int uartfifo_write(const devfs_handle_t *handle, devfs_async_t *wop);
int uartfifo_close(const devfs_handle_t *handle);

#define UARTFIFO_DECLARE_CONFIG_STATE(                                                   \
  uart_fifo_name, uart_fifo_buffer_size, uart_fifo_read_buffer_size, uart_attr_flags,    \
  uart_attr_width, uart_attr_freq, uart_attr_tx_port, uart_attr_tx_pin,                  \
  uart_attr_rx_port, uart_attr_rx_pin, uart_attr_cts_port, uart_attr_cts_pin,            \
  uart_attr_rts_port, uart_attr_rts_pin)                                                 \
  uartfifo_state_t uart_fifo_name##_state MCU_SYS_MEM;                                   \
  char uart_fifo_name##_buffer[uart_fifo_buffer_size] MCU_SYS_MEM;                       \
  char uart_fifo_name##_read_buffer[uart_fifo_read_buffer_size] MCU_SYS_MEM;             \
  const uartfifo_config_t uart_fifo_name##_config = {                                    \
    .uart =                                                                              \
      {.attr = {UART_DEFINE_ATTR(                                                        \
         uart_attr_flags, uart_attr_width, uart_attr_freq, uart_attr_tx_port,            \
         uart_attr_tx_pin, uart_attr_rx_port, uart_attr_rx_pin, uart_attr_cts_port,      \
         uart_attr_cts_pin, uart_attr_rts_port, uart_attr_rts_pin)}},                    \
    .fifo = {.size = uart_fifo_buffer_size, .buffer = uart_fifo_name##_buffer},          \
    .read_buffer = uart_fifo_name##_read_buffer,                                         \
    .read_buffer_size = uart_fifo_read_buffer_size}

#ifdef __cplusplus
}
#endif

#endif /* DEV_UARTFIFO_H_ */
