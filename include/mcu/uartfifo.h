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

#ifndef DEV_UARTFIFO_H_
#define DEV_UARTFIFO_H_

#include "sos/dev/uartfifo.h"

#include <stdbool.h>
#include "fifo.h"



/*! \details This stores the data for the state of the fifo buffer.
 *
 */
typedef struct {
	fifo_state_t fifo;
} uartfifo_state_t;

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
#define UARTFIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(uartfifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}


#define UARTFIFO_DEVICE_CFG(target_port, target_buffer, buffer_size) { \
	.port = target_port, \
	.fifo.buffer = target_buffer, \
	.fifo.size = buffer_size, \
}


/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
	int port /*! The UART port associated with the device */;
	fifo_cfg_t fifo;
} uartfifo_cfg_t;

#ifdef __cplusplus
extern "C" {
#endif

int uartfifo_open(const devfs_handle_t * cfg);
int uartfifo_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int uartfifo_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int uartfifo_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int uartfifo_close(const devfs_handle_t * cfg);

#ifdef __cplusplus
}
#endif


#endif /* DEV_UARTFIFO_H_ */

