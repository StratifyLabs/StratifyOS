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

/*! \addtogroup SYS_FIFO FIFO Buffer
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device is a FIFO RAM buffer.  A write to the buffer adds
 * data to the FIFO.  A read from the buffer reads from the FIFO.  If the FIFO
 * fills up, additional writes will block until data arrives.  Similarly, if the FIFO
 * is empty, a read will block until data arrives.
 *
 */

/*! \file  */

#ifndef IFACE_DEV_VCP_STDOUT_H_
#define IFACE_DEV_VCP_STDOUT_H_

#include <sys/types.h>
#include <stdint.h>
#include "iface/dev/ioctl.h"
#include "mcu/types.h"

#define FIFO_IOC_CHAR 'f'

/*! \brief See below.
 * \details This defines the configuration values for a FIFO device.
 *
 * \param device_name The name of the device (e.g "fifo0")
 * \param cfg_ptr A pointer to the const \ref fifo_cfg_t data structure
 * \param state_ptr A pointer to the state structure (see \ref fifo_state_t)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 * Example:
 * \code
 * #include <hwdl/fifo.h>
 * const fifo_cfg_t fifo_cfg0 = { .buffer = fifo_buf0, .size = FIFO_BUF_SIZE };
 * fifo_state_t fifo_state0;
 * const device_t devices[DEVICES_TOTAL+1] = {
 * 	...
 * 	FIFO_DEVICE("fifo0", &fifo_cfg0, &fifo_state0, 0666, USER_ROOT, GROUP_ROOT),
 * 	...
 * 	DEVICE_TERMINATOR
 * }
 * \endcode
 *
 */

#define VCP_STDOUT_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(vcp_stdout), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

#define VCP_STDOUT_DEVICE_CFG(buf, buf_size, port, int_end, iface) { \
		.fifo_cfg.buffer = buf, \
		.fifo_cfg.size = buf_size, \
		.usb_port = port, \
		.usb_interrupt_endpoint = int_end, \
		.usb_interface = iface \
}


#endif /* IFACE_DEV_FIFO_H_ */


/*! @} */
