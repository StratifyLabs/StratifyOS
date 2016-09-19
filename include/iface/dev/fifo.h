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

#ifndef IFACE_DEV_FIFO_H_
#define IFACE_DEV_FIFO_H_

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

#define FIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(fifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}


#define FIFO_DEVICE_CFG(buf, buf_size) { .buffer = buf, .size = buf_size }

/*! \brief FIFO Attributes
 * \details This structure defines the attributes of a FIFO.
 *  The attributes are read-only using \ref I_FIFO_ATTR.
 */
typedef struct MCU_PACK {
	uint32_t size /*! \brief The total number of bytes in the FIFO */;
	uint32_t used /*! \brief The number of bytes ready to be read from the FIFO */;
	uint32_t overflow /*! \brief If non-zero, indicates data received in the buffer was discarded */;
} fifo_attr_t;

/*! \brief FIFO Peek data */
/*! \details This is used when peeking at data in the FIFO */
typedef struct MCU_PACK {
	ssize_t loc /*! \brief The location to peek */;
	char * buf /*! \brief A pointer to the buffer to receive peek bytes */;
	ssize_t nbyte /*! \brief The number of bytes to peek */;
} fifo_peek_t;

/*! \brief See below.
 * \details This request gets the size of the FIFO in bytes.
 * Example:
 * \code
 * fifo_attr_t attr;
 * ioctl(fifo_fd, I_FIFO_ATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_FIFO_ATTR _IOCTLR(FIFO_IOC_CHAR, 0, fifo_attr_t)
#define I_FIFO_GETATTR I_FIFO_ATTR


/*! \brief This request flushes all data out of the fifo (third ioctl() arg is omitted).
 */
#define I_FIFO_FLUSH _IOCTL(FIFO_IOC_CHAR, 1)

/*! \brief This request initializes the fifo (third ioctl() arg is omitted).
 */
#define I_FIFO_INIT _IOCTL(FIFO_IOC_CHAR, 2)

/*! \brief This request shuts down the operation of the FIFO.
 */
#define I_FIFO_EXIT _IOCTL(FIFO_IOC_CHAR, 3)

/*! \brief This request tells the FIFO whether or not
 * to block when data is written and the buffer is full.
 *
 */
#define I_FIFO_SETWRITEBLOCK _IOCTL(FIFO_IOC_CHAR, 4)

/*! \brief Request a peek at the data in the FIFO.
 * \details This request allows a process to peek
 * at the data in the FIFO without reading it.
 *
 * \code
 * char buffer[16];
 * fifo_peek_t peek_req;
 * peek_req.buf = buffer; //Destination buffer
 * peek_req.loc = 0; //first available bytes in FIFO
 * peek_req.nbyte = 16; //peek up to 16 bytes
 * ioctl(fifo_fd, I_FIFO_PEEK, &peek);
 * \endcode
 *
 */
#define I_FIFO_PEEK _IOCTLR(FIFO_IOC_CHAR, 5, fifo_peek_t)

#define I_FIFO_TOTAL 6


#endif /* IFACE_DEV_FIFO_H_ */


/*! @} */
