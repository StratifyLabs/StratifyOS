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

#ifndef IFACE_DEV_FFIFO_H_
#define IFACE_DEV_FFIFO_H_

#include <sys/types.h>
#include <stdint.h>
#include "iface/dev/ioctl.h"
#include "mcu/types.h"
#include "fifo.h"

#define FFIFO_IOC_CHAR 'F'

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
 * #include <iface/dev/ffifo.h>
 * const ffifo_cfg_t ffifo_cfg0 = { .buffer = fifo_buf0, .size = FFIFO_BUF_SIZE };
 * ffifo_state_t ffifo_state0;
 * const device_t devices[DEVICES_TOTAL+1] = {
 * 	...
 * 	FFIFO_DEVICE("ffifo0", &ffifo_cfg0, &ffifo_state0, 0666, USER_ROOT, GROUP_ROOT),
 * 	...
 * 	DEVICE_TERMINATOR
 * }
 * \endcode
 *
 */
#define FFIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(ffifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

#define FFIFO_DEVICE_CFG(buf, fcount, fsize, read_notify, write_notify) { .buffer = buf, .count = fcount, .frame_size = fsize, .notify_on_read = read_notify, .notify_on_write = write_notify }


enum {
	FFIFO_FLAGS_WRITEBLOCK = FIFO_FLAGS_WRITEBLOCK /*! Write block flag */,
	FFIFO_FLAGS_OVERFLOW = FIFO_FLAGS_OVERFLOW /*! Overflow flag */,
	FFIFO_FLAGS_NOTIFY_WRITE = FIFO_FLAGS_NOTIFY_WRITE /*! Notify on write */,
	FFIFO_FLAGS_NOTIFY_READ = FIFO_FLAGS_NOTIFY_READ /*! Notify on read */,
};

/*! \brief FIFO Attributes
 * \details This structure defines the attributes of a FIFO.
 *  The attributes are read-only using \ref I_FIFO_ATTR.
 */
typedef struct MCU_PACK {
	u16 count /*! Total number of frames in the fifo */;
	u16 frame_size /*! Frame size of the fifo */;
	u16 used /*! Number of frames being used */;
	u16 resd;
	u32 o_flags /*! Fifo flags */;
} ffifo_attr_t;

#define I_FFIFO_ATTR _IOCTLR(FIFO_IOC_CHAR, 0, ffifo_attr_t)
#define I_FFIFO_GETATTR I_FFIFO_ATTR

#define I_FFIFO_FLUSH I_FIFO_FLUSH
#define I_FFIFO_INIT I_FIFO_INIT
#define I_FFIFO_EXIT I_FIFO_EXIT
#define I_FFIFO_SETWRITEBLOCK I_FIFO_SETWRITEBLOCK




#endif /* IFACE_DEV_FFIFO_H_ */


/*! @} */
