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

/*! \addtogroup SYS_USBFIFO USB FIFO
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device driver is used to add a FIFO read buffer to a USB port.
 *
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_I2S_FFIFO_H_
#define SOS_DEV_I2S_FFIFO_H_

#include "mcu/types.h"
#include "ffifo.h"
#include "i2s.h"


typedef struct MCU_PACK {
    ffifo_attr_t tx;
    ffifo_attr_t rx;
} i2s_ffifo_attr_t;

typedef struct MCU_PACK {
    ffifo_info_t ffifo;
    u32 count;
    s32 error;
} i2s_ffifo_channel_info_t;

typedef struct MCU_PACK {
    i2s_ffifo_channel_info_t rx;
    i2s_ffifo_channel_info_t tx;
} i2s_ffifo_info_t;


#define I2S_FFIFO_VERSION (0x030000)
#define I2S_FFIFO_IOC_IDENT_CHAR 'j'

#define I_I2S_FFIFO_GETVERSION _IOCTL(I2S_FFIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_I2S_FFIFO_GETINFO _IOCTLR(I2S_FFIFO_IOC_IDENT_CHAR, I_MCU_GETINFO, i2s_ffifo_info_t)
#define I_I2S_FFIFO_SETATTR _IOCTLW(I2S_FFIFO_IOC_IDENT_CHAR, I_MCU_SETATTR, i2s_ffifo_attr_t)
#define I_I2S_FFIFO_SETACTION _IOCTLW(I2S_FFIFO_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#endif /* SOS_DEV_USBFIFO_H_ */


/*! @} */
