// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup USB_FIFO
 * @{
 * \ingroup IFACE_DEV
 *
 */

/*! \file  */

#ifndef DEV_USBFIFO_H_
#define DEV_USBFIFO_H_

#include "fifo.h"
#include "mcu/usb.h"
#include "sos/dev/usbfifo.h"

/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
  fifo_state_t fifo;
  devfs_async_t async_read;
} usbfifo_state_t;

/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
  devfs_device_t device;
  usb_config_t usb;
  int endpoint /*! The USB endpoint number to read */;
  int endpoint_size /*! The USB endpoint data size */;
  fifo_config_t fifo;
  char *read_buffer;
} usbfifo_config_t;

int usbfifo_open(const devfs_handle_t *handle);
int usbfifo_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int usbfifo_read(const devfs_handle_t *handle, devfs_async_t *async);
int usbfifo_write(const devfs_handle_t *handle, devfs_async_t *async);
int usbfifo_close(const devfs_handle_t *handle);

#define USBFIFO_DECLARE_CONFIG_STATE(                                                    \
  usb_fifo_name, usb_fifo_buffer_size, usb_attr_endpoint, usb_attr_endpoint_size)        \
  usbfifo_state_t usb_fifo_name##_state MCU_SYS_MEM;                                     \
  char usb_fifo_name##_read_buffer[usb_attr_endpoint_size];                              \
  char usb_fifo_name##_buffer[usb_fifo_buffer_size] MCU_SYS_MEM;                         \
  const usbfifo_config_t usb_fifo_name##_config = {                                      \
    .endpoint = usb_attr_endpoint,                                                       \
    .endpoint_size = usb_attr_endpoint_size,                                             \
    .read_buffer = usb_fifo_name##_read_buffer,                                          \
    .fifo = {.size = usb_fifo_buffer_size, .buffer = usb_fifo_name##_buffer}}

#endif /* DEV_USBFIFO_H_ */

/*! @} */
