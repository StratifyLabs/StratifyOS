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

/*! \addtogroup QSPI Quad Serial Peripheral Interface (QSPI)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details This module implements a hardware Quad SPI master.
 *
 *
 *
 *
 */

/*! \file
 * \brief Serial Peripheral Interface Header File
 *
 */

#ifndef SOS_DEV_QSPI_H_
#define SOS_DEV_QSPI_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define QSPI_VERSION (0x030000)
#define QSPI_IOC_IDENT_CHAR 'Q'

typedef enum {
	QSPI_FLAG_SET_MASTER /*! Configure the QSPI as master */ = (1<<0),
} qspi_flag_t;

typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask of supported flags */;
	u32 o_events /*! Bitmask of supported events */;
	u32 resd[8];
} qspi_info_t;

/*! \brief QSPI Pin Assignment Data
 *
 */
typedef struct MCU_PACK {
	mcu_pin_t data[4] /*! Up to four data lines */;
	mcu_pin_t sck /*! Serial clock pin */;
	mcu_pin_t cs /*! Chip select pin */;
} qspi_pin_assignment_t;


/*! \brief QSPI Set Attributes Data
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Flag bitmask */;
	qspi_pin_assignment_t pin_assignment /*! Pin Assignment */;
	u32 freq /*! Target operating frequency */;
	u8 width /*! Width for transactions */;
	u32 size /*! Size of the flash memory */;
	u32 resd[8];
} qspi_attr_t;

#define I_QSPI_GETVERSION _IOCTL(QSPI_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief This request gets the SPI attributes.
 * \hideinitializer
 */
#define I_QSPI_GETINFO _IOCTLR(QSPI_IOC_IDENT_CHAR, I_MCU_GETINFO, qspi_info_t)

/*! \brief This request sets the SPI attributes.
 * \hideinitializer
 */
#define I_QSPI_SETATTR _IOCTLW(QSPI_IOC_IDENT_CHAR, I_MCU_SETATTR, qspi_attr_t)
#define I_QSPI_SETACTION _IOCTLW(QSPI_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)



#define I_QSPI_TOTAL 0

#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_QSPI_H_ */

/*! @} */
