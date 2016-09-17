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

/*! \addtogroup USB_FIFO
 * @{
 * \ingroup IFACE_DEV
 *
 */

/*! \file  */

#ifndef DEV_USBFIFO_H_
#define DEV_USBFIFO_H_

#include <stdbool.h>
#include "iface/dev/usbfifo.h"
#include "mcu/circ_buf.h"
#include "fifo.h"



/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
	fifo_state_t fifo /*! Fifo for receiving data. Writes are not buffered */;
} usbfifo_state_t;

/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
	int port /*! The USB port associated with the device */;
	int endpoint /*! The USB endpoint number to read */;
	int endpoint_size /*! The USB endpoint number to read */;
	fifo_cfg_t fifo /*! Fifo data for receving data */;
} usbfifo_cfg_t;

int usbfifo_open(const device_cfg_t * cfg);
int usbfifo_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int usbfifo_read(const device_cfg_t * cfg, device_transfer_t * rop);
int usbfifo_write(const device_cfg_t * cfg, device_transfer_t * wop);
int usbfifo_close(const device_cfg_t * cfg);


#endif /* DEV_USBFIFO_H_ */


/*! @} */
