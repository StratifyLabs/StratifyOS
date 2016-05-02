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

/*! \addtogroup SYS_DEVFIFO Device FIFO
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device driver is used to add a FIFO read buffer to a device.  When writing the FIFO,
 * the device is written directly.  Any data that arrives asynchronously through the device is stored
 * in the FIFO buffer.  When reading the FIFO, the FIFO buffer is read.
 *
 *
 *
 *
 */

/*! \file  */

#ifndef DEV_DEVFIFO_H_
#define DEV_DEVFIFO_H_

#include <stdint.h>
#include "iface/dev/ioctl.h"
#include "iface/device_config.h"
#include "mcu/types.h"


#define DEVFIFO_IOC_CHAR 'f'

/*! \details This defines the configuration values for a FIFO that
 * is connected to an underlying device.
 *
 * \param device_name The name of the device (e.g "uart-fifo")
 * \param cfg_ptr A pointer to the const \ref devfifo_cfg_t data structure
 * \param state_ptr A pointer to the state structure (see \ref devfifo_state_t)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 * Example:
 * \code
 * const device_t devices[DEVICES_TOTAL+1] = {
 *	DEVICE_PERIPH("uart0", uart, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
 *	DEVFIFO_DEVICE("uart0-fifo", &devices[0], 0666, USER_ROOT, GROUP_ROOT),
 * 	...
 * 	DEVICE_TERMINATOR
 * }
 * \endcode
 *
 */
#define DEVFIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(devfifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}


#define DEVFIFO_DEVICE_CFG(target_device, target_buffer, buffer_size, getbyte, setaction, callback_event) { \
	.dev = target_device, \
	.buffer = target_buffer, \
	.size = buffer_size, \
	.req_getbyte = getbyte, \
	.req_setaction = setaction, \
	.event = callback_event \
}

/*! \details This structure defines the attributes used
 * in a device fifo.
 */
typedef struct MCU_PACK {
	uint32_t size /*! \brief The total number of bytes in the FIFO */;
	uint32_t used /*! \brief The number of bytes ready to be read from the FIFO */;
	uint32_t overflow /*! \brief Non-zero if FIFO has dropped data because data is arriving
	faster than it is being read; this value is cleared when it is read using I_DEVFIFO_GETATTR */;
} devfifo_attr_t;

/*! \details This request gets the size of the FIFO in bytes.
 *
 * Example:
 * \code
 * devfifo_attr_t attr;
 * ioctl(fifo_fd, I_DEVFIFO_GETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_DEVFIFO_GETATTR _IOCTLR(DEVFIFO_IOC_CHAR, 0, devfifo_attr_t)

#define I_DEVFIFO_TOTAL 1

/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
	const device_t * dev /*! \brief The underlying device */;
	char * buffer /*! \brief The buffer for the fifo */;
	int size /*! \brief The size of the fifo */;
	int req_getbyte /*! \brief The request used to get a byte from the device */;
	int req_setaction /*! \brief The request to set the action */;
	int event /*! \brief The event to trigger on */;
} devfifo_cfg_t;


#endif /* DEV_DEV_FIFO_H_ */


/*! @} */
