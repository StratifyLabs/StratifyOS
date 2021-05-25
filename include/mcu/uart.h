// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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
  uart_attr_t attr; // default attributes
  u32 port;
  const void *fifo_config;
} uart_config_t;

#define MCU_UART_IOCTL_REQUEST_DECLARATION(driver_name)                                  \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);                         \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, get);                               \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, put);                               \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, flush)

#define MCU_UART_DRIVER_DECLARATION(variant)                                             \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_UART_IOCTL_REQUEST_DECLARATION(variant)

MCU_UART_DRIVER_DECLARATION(mcu_uart);
MCU_UART_DRIVER_DECLARATION(mcu_uart_dma);

#define UART_DEFINE_ATTR(                                                                \
  uart_attr_flags, uart_attr_width, uart_attr_freq, uart_attr_tx_port, uart_attr_tx_pin, \
  uart_attr_rx_port, uart_attr_rx_pin, uart_attr_rts_port, uart_attr_rts_pin,            \
  uart_attr_cts_port, uart_attr_cts_pin)                                                 \
  .o_flags = uart_attr_flags, .width = uart_attr_width, .freq = uart_attr_freq,          \
  .pin_assignment.tx = {uart_attr_tx_port, uart_attr_tx_pin},                            \
  .pin_assignment.rx = {uart_attr_rx_port, uart_attr_rx_pin},                            \
  .pin_assignment.rts = {uart_attr_rts_port, uart_attr_rts_pin},                         \
  .pin_assignment.cts = {uart_attr_cts_port, uart_attr_cts_pin}

#define UART_DECLARE_CONFIG(                                                             \
  uart_name, uart_attr_flags, uart_attr_width, uart_attr_freq, uart_attr_tx_port,        \
  uart_attr_tx_pin, uart_attr_rx_port, uart_attr_rx_pin, uart_attr_rts_port,             \
  uart_attr_rts_pin, uart_attr_cts_port, uart_attr_cts_pin)                              \
  const uart_config_t uart_name##config = {                                              \
    .attr = {UART_DEFINE_ATTR(                                                           \
      uart_attr_flags, uart_attr_width, uart_attr_freq, uart_attr_tx_port,               \
      uart_attr_tx_pin, uart_attr_rx_port, uart_attr_rx_pin, uart_attr_rts_port,         \
      uart_attr_rts_pin, uart_attr_cts_port, uart_attr_cts_pin)}}

#ifdef __cplusplus
}
#endif

#endif // _MCU_UART_H_

/*! @} */
