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

#ifndef SOS_DEV_I2S_H_
#define SOS_DEV_I2S_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2S_VERSION (0x030000)
#define I2S_IOC_IDENT_CHAR 'I'

enum {
	I2S_FLAG_IS_WIDTH_8 /*! I2S Word Width 8 bits */ = (1<<0),
	I2S_FLAG_IS_WIDTH_16 /*! I2S Word Width 16 bits */ = (1<<1),
	I2S_FLAG_IS_WIDTH_24 /*! I2S Word Width 24 bits */ = (1<<2),
	I2S_FLAG_IS_WIDTH_32 /*! I2S Word Width 32 bits */ = (1<<3),
	I2S_FLAG_IS_MONO /*! I2S Mono mode */ = (1<<4),
	I2S_FLAG_IS_STEREO /*! I2S Stereo mode (default behavoir) */ = (1<<5),
	I2S_FLAG_SET_MASTER /*! Set the I2S as a master */ = (1<<6),
	I2S_FLAG_SET_SLAVE /*! Set the I2S as a slave */ = (1<<7),
	I2S_FLAG_IS_TRANSMITTER /*! Set the I2S transmitter (master or slave) */ = (1<<8),
	I2S_FLAG_IS_RECEIVER /*! Set the I2S receiver (master or slave) */ = (1<<9),
	I2S_FLAG_IS_MCK_ENABLED /*! Set this bit to enable the mclk output */ = (1<<12),
};

/*! \brief I2S IO Attributes
 *  \details This structure defines how the control structure
 * for configuring the I2S port.
 */
typedef struct MCU_PACK {
	u32 o_flags  /*!  Mode flags (see I2S_MODE_*) */;
	u32 o_events  /*! Mode flags (see I2S_MODE_*) */;
	u32 freq /*! The I2S audio frequency */;
	u32 resd[8];
} i2s_info_t;


typedef struct MCU_PACK {
	mcu_pin_t ws;
	mcu_pin_t sck;
	mcu_pin_t sdout;
	mcu_pin_t sdin;
	mcu_pin_t mck;
} i2s_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags  /*! Flag bitmask */;
	u32 freq /*! The I2S audio frequency */;
	i2s_pin_assignment_t pin_assignment /*! The pin assignement values */;
	u32 mck_mult /*! The I2S mclk multiplier value */;
	u32 resd[8];
} i2s_attr_t;

#define I_I2S_GETVERSION _IOCTL(I2S_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief This request gets the I2S attributes.
 * \hideinitializer
 */
#define I_I2S_GETINFO _IOCTLR(I2S_IOC_IDENT_CHAR, I_MCU_GETINFO, i2s_info_t)

/*! \brief This request sets the I2S attributes.
 * \hideinitializer
 */
#define I_I2S_SETATTR _IOCTLW(I2S_IOC_IDENT_CHAR, I_MCU_SETATTR, i2s_attr_t)

/*! \brief This request sets the I2S action.
 * \hideinitializer
 */
#define I_I2S_SETACTION _IOCTLW(I2S_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#define I_I2S_MUTE _IOCTL(I2S_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)
#define I_I2S_UNMUTE _IOCTL(I2S_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)


#define I_I2S_TOTAL 2


#ifdef __cplusplus
}
#endif

#endif // SOS_DEV_I2S_H_

/*! @} */

