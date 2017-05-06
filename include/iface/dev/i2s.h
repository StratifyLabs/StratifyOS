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

/*! \addtogroup I2S Inter-Integrated Sound (I2S) Master
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module implements an I2S master.
 *
 */

/*! \file
 * \brief I2S Header File
 *
 */

#ifndef IFACE_DEV_I2S_H_
#define IFACE_DEV_I2S_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2S_IOC_IDENT_CHAR 'I'


enum {
	I2S_MODE_WORDWIDTH_8 /*! \brief I2S Word Width 8 bits */ = (1<<0),
	I2S_MODE_WORDWIDTH_16 /*! \brief I2S Word Width 16 bits */ = (1<<1),
	I2S_MODE_WORDWIDTH_32 /*! \brief I2S Word Width 32 bits */ = (1<<2),
	I2S_MODE_MONO = (1<<3),
	I2S_MODE_STEREO = (1<<4),
	I2S_MODE_MASTER = (1<<5),
	I2S_MODE_OUTPUT = (1<<6) /*! \brief Configure I2S output */,
	I2S_MODE_INPUT = (1<<7) /*! \brief Configure I2S input (independent from output) */,
	I2S_MODE_MCLK_ENABLE = (1<<8) /*! Set this bit to enable the mclk output */,
	I2S_MODE_DUPLEX = (1<<9) /*! Set this bit to both read and write audio data at the same time */
};

enum {
	I2S_ACTION_EVENT_READ = (1<<0),
	I2S_ACTION_EVENT_WRITE = (1<<1)
};



/*! \brief I2S IO Attributes
 *  \details This structure defines how the control structure
 * for configuring the I2S port.
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	u8 resd[3];
	u32 o_mode  /*! \brief Mode flags (see I2S_MODE_*) */;
	u32 frequency /*! \brief The I2S audio frequency */;
} i2s_attr_t;


/*! \details This defines an I2S action.
 */
typedef mcu_action_t i2s_action_t;

/*! \brief This request gets the I2S attributes.
 * \hideinitializer
 */
#define I_I2S_ATTR _IOCTLR(I2S_IOC_IDENT_CHAR, I_GLOBAL_ATTR, i2s_attr_t)
#define I_I2S_GETATTR I_I2S_ATTR

/*! \brief This request sets the I2S attributes.
 * \hideinitializer
 */
#define I_I2S_SETATTR _IOCTLW(I2S_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, i2s_attr_t)

/*! \brief This request sets the I2S action.
 * \hideinitializer
 */
#define I_I2S_SETACTION _IOCTLW(I2S_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, i2s_action_t)


#define I_I2S_MUTE _IOCTL(I2S_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)
#define I_I2S_UNMUTE _IOCTL(I2S_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2)


#define I_I2S_TOTAL 2


#ifdef __cplusplus
}
#endif

#endif // IFACE_DEV_I2S_H_

/*! @} */

