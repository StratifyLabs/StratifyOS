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

/*! \addtogroup TOUCH Touch
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing touch screen devices.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_TOUCH_H_
#define SOS_DEV_TOUCH_H_

#include "mcu/types.h"

#define TOUCH_VERSION (0x030000)
#define TOUCH_IOC_IDENT_CHAR 't'

enum {
	TOUCH_FLAG_INIT /*! Initializes the touch display. */ = (1<<0),
	TOUCH_FLAG_RESET /*! Issue a reset to the touch display. */ = (1<<1),
	TOUCH_FLAG_IS_MULTITOUCH /*! Initialize in multitouch mode if supported. */ = (1<<2),
} touch_flags_t;

/*! \brief Touch Info
 * \details This is the data structure for accessing
 * information about the device.
 *
 */
typedef struct MCU_PACK {

	u32 resd[8];
} touch_info_t;


/*! \brief Touch Attributes
 * \details This is the data structure used with setting attributes.
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Drive flags such as \ref TOUCH_FLAG_INIT */;

	u32 resd[8];
} touch_attr_t;

/*! \details Gets the driver version. */
#define I_TOUCH_GETVERSION _IOCTL(TOUCH_IOC_IDENT_CHAR, I_MCU_GETVERSION)
/*! \details Gets the touch info (touch_info_t). */
#define I_TOUCH_GETINFO _IOCTLR(TOUCH_IOC_IDENT_CHAR, I_MCU_GETINFO, touch_info_t)
/*! \details Sets the touch display attributes (\sa touch_attr_t). */
#define I_TOUCH_SETATTR _IOCTLW(TOUCH_IOC_IDENT_CHAR, I_MCU_SETATTR, touch_attr_t)
#define I_TOUCH_SETACTION _IOCTLW(TOUCH_IOC_IDENT_CHAR, I_MCU_SETATTR, mcu_action_t)


#define I_TOUCH_TOTAL 0

#endif /* SOS_DEV_TOUCH_H_ */

/*! @} */
