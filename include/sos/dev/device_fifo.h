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

/*! \addtogroup DEVICE_FIFO Device FIFO
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_DEVICE_FIFO_H_
#define SOS_DEV_DEVICE_FIFO_H_

#include <sdk/types.h>
#include "ffifo.h"


enum {
	DEVICE_FIFO_FLAG_START /*! Start streaming */= (1<<0),
	DEVICE_FIFO_FLAG_STOP /*! Stop streaming */ = (1<<1),
	DEVICE_FIFO_FLAG_FLUSH /*! Flush all stream data */ = (1<<2)
};

typedef struct MCU_PACK {
	u32 o_flags /*! Use with DEVICE_FIFO_FLAG_START or DEVICE_FIFO_FLAG_STOP */;
	fifo_attr_t tx;
	fifo_attr_t rx;
} device_fifo_attr_t;

typedef struct MCU_PACK {
	fifo_info_t fifo;
	u32 count;
	s32 error;
} device_fifo_channel_info_t;

typedef struct MCU_PACK {
	device_fifo_channel_info_t rx;
	device_fifo_channel_info_t tx;
} device_fifo_info_t;


#define DEVICE_FIFO_VERSION (0x030000)
#define DEVICE_FIFO_IOC_IDENT_CHAR 'd'

#define I_DEVICE_FIFO_GETVERSION _IOCTL(DEVICE_FIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_DEVICE_FIFO_GETINFO _IOCTLR(DEVICE_FIFO_IOC_IDENT_CHAR, I_MCU_GETINFO, device_fifo_info_t)
#define I_DEVICE_FIFO_SETATTR _IOCTLW(DEVICE_FIFO_IOC_IDENT_CHAR, I_MCU_SETATTR, device_fifo_attr_t)
#define I_DEVICE_FIFO_SETACTION _IOCTLW(DEVICE_FIFO_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#endif /* SOS_DEV_DEVICE_FIFO_H_ */


/*! @} */
