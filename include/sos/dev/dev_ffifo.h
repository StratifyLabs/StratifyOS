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

#ifndef SOS_DEV_DEV_FFIFO_H_
#define SOS_DEV_DEV_FFIFO_H_

#include "mcu/types.h"
#include "ffifo.h"

typedef struct MCU_PACK {
    ffifo_attr_t tx;
    ffifo_attr_t rx;
} dev_ffifo_attr_t;

typedef struct MCU_PACK {
    ffifo_info_t ffifo;
    u32 count;
    s32 error;
} dev_ffifo_channel_info_t;

typedef struct MCU_PACK {
    dev_ffifo_channel_info_t rx;
    dev_ffifo_channel_info_t tx;
} dev_ffifo_info_t;


#define DEV_FFIFO_VERSION (0x030000)
#define DEV_FFIFO_IOC_IDENT_CHAR 'k'

#define I_DEV_FFIFO_GETVERSION _IOCTL(DEV_FFIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_DEV_FFIFO_GETINFO _IOCTLR(DEV_FFIFO_IOC_IDENT_CHAR, I_MCU_GETINFO, dev_ffifo_info_t)
#define I_DEV_FFIFO_SETATTR _IOCTLW(DEV_FFIFO_IOC_IDENT_CHAR, I_MCU_SETATTR, dev_ffifo_attr_t)
#define I_DEV_FFIFO_SETACTION _IOCTLW(DEV_FFIFO_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#endif /* SOS_DEV_DEV_FFIFO_H_ */


/*! @} */
