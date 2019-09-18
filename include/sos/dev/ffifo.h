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

#ifndef SOS_STREAM_FFIFO_H_
#define SOS_STREAM_FFIFO_H_

#include "fifo.h"
#include "mcu/types.h"

#define FFIFO_VERSION (0x030000)
#define FFIFO_IOC_CHAR 'F'

enum {
	FFIFO_FLAG_SET_WRITEBLOCK = FIFO_FLAG_SET_WRITEBLOCK /*! Write block flag */,
	FFIFO_FLAG_IS_OVERFLOW = FIFO_FLAG_IS_OVERFLOW /*! Overflow flag (if set with FFIFO_FLAG_SET_WRITEBLOCK, write block is disabled) */,
	FFIFO_FLAG_NOTIFY_WRITE = FIFO_FLAG_NOTIFY_WRITE /*! Notify on write */,
	FFIFO_FLAG_NOTIFY_READ = FIFO_FLAG_NOTIFY_READ /*! Notify on read */,
};

typedef struct MCU_PACK {
	u32 o_flags /*! Fifo flags */;
	u16 frame_count /*! Total number of frames in the fifo */;
	u16 frame_size /*! Frame size of the fifo */;
	u16 frame_count_ready /*! Number of frames being used */;
	u16 resd_align;
	u32 resd[8];
} ffifo_info_t;


/*! \brief FIFO Attributes
 * \details This structure defines the attributes of a FIFO.
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Fifo flags */;
	u32 resd[8];
} ffifo_attr_t;

#define I_FFIFO_GETVERSION _IOCTL(FFIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_FFIFO_GETINFO _IOCTLR(FIFO_IOC_CHAR, 0, ffifo_info_t)
#define I_FFIFO_SETATTR _IOCTLW(FIFO_IOC_CHAR, 1, ffifo_attr_t)

#define I_FFIFO_FLUSH I_FIFO_FLUSH
#define I_FFIFO_INIT I_FIFO_INIT
#define I_FFIFO_EXIT I_FIFO_EXIT




#endif /* SOS_STREAM_FFIFO_H_ */


/*! @} */
