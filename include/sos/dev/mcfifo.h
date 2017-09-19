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

#ifndef SOS_DEV_MCFIFO_H_
#define SOS_DEV_MCFIFO_H_

#include "fifo.h"
#include "mcu/types.h"

#define MCFIFO_VERSION (0x030000)
#define MCFIFO_IOC_CHAR 'M'

enum {
	MCFIFO_FLAG_NONE = 0,
};

typedef struct MCU_PACK {
	u32 o_flags /*! Fifo flags */;
	u16 count /*! Total number of channels in the fifo */;
	u16 size /*! The size of each channel */;
	u32 o_ready /*! Bitmask of channels with at least one byte */;
	u32 resd[8];
} mcfifo_info_t;


/*! \brief FIFO Attributes
 * \details This structure defines the attributes of a FIFO.
 *  The attributes are read-only using \ref I_FIFO_ATTR.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Fifo flags */;
	u32 resd[8];
} mcfifo_attr_t;

typedef struct MCU_PACK {
	u32 channel;
	int request;
	void * ctl;
} mcfifo_fiforequest_t;

#define I_MCFIFO_GETVERSION _IOCTL(MCFIFO_IOC_CHAR, I_MCU_GETVERSION)
#define I_MCFIFO_GETINFO _IOCTLR(MCFIFO_IOC_CHAR, 0, mcfifo_info_t)
#define I_MCFIFO_SETATTR _IOCTLW(MCFIFO_IOC_CHAR, 1, mcfifo_attr_t)

#define I_MCFIFO_GETOWNER _IOCTLRW(MCFIFO_IOC_CHAR, 2, mcu_channel_t)
#define I_MCFIFO_SETOWNER _IOCTLW(MCFIFO_IOC_CHAR, 3, mcu_channel_t)

#define I_MCFIFO_FIFOREQUEST _IOCTLRW(MCFIFO_IOC_CHAR, 4, mcfifo_fiforequest_t)




#endif /* SOS_DEV_MCFIFO_H_ */


/*! @} */
