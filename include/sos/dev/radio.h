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

/*! \addtogroup RADIO Radio
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing radios (such as ISM band radios).
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_RADIO_H_
#define SOS_DEV_RADIO_H_


#include <stdint.h>

#include "ioctl.h"
#include "mcu/types.h"

#define RADIO_IOC_CHAR 'R'

enum {
	RADIO_MODE_STANDBY,
	RADIO_MODE_SLEEP,
	RADIO_MODE_IDLE,
	RADIO_MODE_TX,
	RADIO_MODE_RX,

};

/*! \brief Radio attributes
 */
typedef struct MCU_PACK {

	//state handling
	u8 mode;
	u8 resd;

	//packet attributes
	u16 preamble_len;
	u16 fixed_len;
	u16 payload_len;

	//radio attributes
	u32 code_rate /*! LoRa Code rate */;
	u32 data_rate /*! Radio data rate */;
	u32 freq /*! Radio Frequency */;
	u32 * hopping_table /*! A pointer to the frequency hopping table */;
	u16 hopping_entries /*! The number of entries in the frequency hopping table */;
	u16 power /*! The power output */;
} radio_attr_t;


/*! \details This request reads the radio attributes.
 *
 * Example:
 * \code
 * ioctl(fildes, I_RADIO_GETINFO);
 * \endcode
 * \hideinitializer
 */
#define I_RADIO_GETINFO _IOCTLR(RADIO_IOC_CHAR, 0, radio_attr_t)

/*! \details This request sets the radio attributes.
 *
 * Example:
 * \code
 * ioctl(fildes, I_RADIO_SETATTR);
 * \endcode
 * \hideinitializer
 */
#define I_RADIO_SETATTR _IOCTLW(RADIO_IOC_CHAR, 1, radio_attr_t)

/*! \details This request sets the radio attributes.
 *
 * Example:
 * \code
 * ioctl(fildes, I_RADIO_SETATTR);
 * \endcode
 * \hideinitializer
 */
#define I_RADIO_SETFREQUENCY _IOCTL(RADIO_IOC_CHAR, 2)

/*! \details This request sets the radio mode
 *
 * Example:
 * \code
 * ioctl(fildes, I_RADIO_SETMODE, RADIO_MODE_TX);
 * \endcode
 * \hideinitializer
 */
#define I_RADIO_SETMODE _IOCTL(RADIO_IOC_CHAR, 3)

/*! \details This request sets the radio packet size.
 *
 * Example:
 * \code
 * ioctl(fildes, I_RADIO_SETPACKET_SIZE, 24);
 * \endcode
 * \hideinitializer
 */
#define I_RADIO_SETPACKETSIZE _IOCTL(RADIO_IOC_CHAR, 4)


#define I_RADIO_TOTAL 5

#endif /* SOS_DEV_RADIO_H_ */

/*! @} */
