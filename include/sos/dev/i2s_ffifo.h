// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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

/*! \file 
 */

#ifndef SOS_DEV_I2S_FFIFO_H_
#define SOS_DEV_I2S_FFIFO_H_

#include <sdk/types.h>
#include "ffifo.h"
#include "i2s.h"


typedef struct MCU_PACK {
	ffifo_attr_t tx;
	ffifo_attr_t rx;
} i2s_ffifo_attr_t;

typedef struct MCU_PACK {
	ffifo_info_t ffifo;
	u32 access_count;
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
