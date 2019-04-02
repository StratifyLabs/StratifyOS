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

#ifndef DEV_USBFIFO_H_
#define DEV_USBFIFO_H_

#include "sos/dev/usbfifo.h"
#include "mcu/usb.h"
#include "fifo.h"



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
	usb_config_t usb;
	int endpoint /*! The USB endpoint number to read */;
	int endpoint_size /*! The USB endpoint data size */;
	fifo_config_t fifo;
	char * read_buffer;
} usbfifo_config_t;

int usbfifo_open(const devfs_handle_t * handle);
int usbfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int usbfifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int usbfifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int usbfifo_close(const devfs_handle_t * handle);

#define USBFIFO_DECLARE_CONFIG_STATE(usb_fifo_name,\
	usb_fifo_buffer_size, \
	usb_attr_endpoint, \
	usb_attr_endpoint_size ) \
	usbfifo_state_t usb_fifo_name##_state MCU_SYS_MEM; \
	char usb_fifo_name##_read_buffer[usb_attr_endpoint_size]; \
	char usb_fifo_name##_buffer[usb_fifo_buffer_size] MCU_SYS_MEM; \
	const usbfifo_config_t usb_fifo_name##_config = { \
	.endpoint = usb_attr_endpoint, \
	.endpoint_size = usb_attr_endpoint_size, .read_buffer = usb_fifo_name##_read_buffer \
	.fifo = { .size = usb_fifo_buffer_size, .buffer = usb_fifo_name##_buffer } \
	}

#endif /* DEV_USBFIFO_H_ */


/*! @} */
