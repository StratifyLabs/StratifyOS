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

#ifndef DEV_VCP_STDOUT_H_
#define DEV_VCP_STDOUT_H_

#include <stdbool.h>
#include "fifo.h"
#include "iface/dev/vcp_stdout.h"
#include "mcu/circ_buf.h"
#include "stratify/usb_dev_typedefs.h"


/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct MCU_PACK {
	fifo_state_t fifo_state; //this must be first
	device_transfer_t int_op;
	usb_cdc_acm_notification_serialstate_t serial_state;
} vcp_stdout_state_t;

/*! \details This is used for the configuration of the device.
 *
 */
typedef struct MCU_PACK {
	fifo_cfg_t fifo_cfg; //this must be first!
	u8 usb_port;
	u8 usb_interrupt_endpoint;
	u8 usb_interface;
	u8 resd;
} vcp_stdout_cfg_t;

int vcp_stdout_open(const device_cfg_t * cfg);
int vcp_stdout_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int vcp_stdout_read(const device_cfg_t * cfg, device_transfer_t * rop);
int vcp_stdout_write(const device_cfg_t * cfg, device_transfer_t * wop);
int vcp_stdout_close(const device_cfg_t * cfg);


#endif /* DEV_USBFIFO_H_ */


/*! @} */
