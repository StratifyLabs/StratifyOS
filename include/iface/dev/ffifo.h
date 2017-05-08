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
