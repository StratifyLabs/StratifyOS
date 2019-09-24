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

/*! \addtogroup STREAM_FFIFO Stream FFIFO
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details The stream FFIFO device driver is designed to read/write hardware with
 * streaming data that has hard real-time requirements such as audio streaming data
 * from either I2S sources/sinks or DAC/ADC devices.
 *
 * FFIFO stands for framed fifo. The driver assumes the underlying
 * device reads/write data in a circular fashion and provides
 * events for transfers and half transfers.  In this case the frame count,
 * should be set to two.
 *
 * The stream FFIFO supports blocking reads/write. A write() will block
 * until there is space in the buffer to write another frame. A read() will
 * block until data is available. The frames must be handled before that
 * hardware finishes the next frame, otherwise overrun/underrun conditions will occur.
 *
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_STREAM_FFIFO_H_
#define SOS_DEV_STREAM_FFIFO_H_

#include "mcu/types.h"
#include "ffifo.h"


enum {
	STREAM_FFIFO_FLAG_START /*! Start streaming */= (1<<0),
	STREAM_FFIFO_FLAG_STOP /*! Stop streaming */ = (1<<1),
	STREAM_FFIFO_FLAG_FLUSH /*! Flush all stream data */ = (1<<2)
};

typedef struct MCU_PACK {
	volatile u32 o_flags /*! Use with STREAM_FFIFO_FLAG_START or STREAM_FFIFO_FLAG_STOP */;
	ffifo_attr_t tx;
	ffifo_attr_t rx;
} stream_ffifo_attr_t;

typedef struct MCU_PACK {
	ffifo_info_t ffifo;
	u32 access_count;
	s32 error;
} stream_ffifo_channel_info_t;

typedef struct MCU_PACK {
	stream_ffifo_channel_info_t rx;
	stream_ffifo_channel_info_t tx;
	u32 o_flags;
	u32 o_status;
} stream_ffifo_info_t;


#define STREAM_FFIFO_VERSION (0x030000)
#define STREAM_FFIFO_IOC_IDENT_CHAR 'S'

#define I_STREAM_FFIFO_GETVERSION _IOCTL(STREAM_FFIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_STREAM_FFIFO_GETINFO _IOCTLR(STREAM_FFIFO_IOC_IDENT_CHAR, I_MCU_GETINFO, stream_ffifo_info_t)
#define I_STREAM_FFIFO_SETATTR _IOCTLW(STREAM_FFIFO_IOC_IDENT_CHAR, I_MCU_SETATTR, stream_ffifo_attr_t)
#define I_STREAM_FFIFO_SETACTION _IOCTLW(STREAM_FFIFO_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#endif /* SOS_DEV_STREAM_FFIFO_H_ */


/*! @} */
