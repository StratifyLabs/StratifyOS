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
 * \brief Quad Serial Peripheral Interface Header File
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


/*! \details QSPI flags used with
 * qspi_attr_t.o_flags and I_QSPI_SETATTR.
 *
 */
typedef enum {
	QSPI_FLAG_SET_MASTER /*! Configure the QSPI as master */ = (1<<0),
	QSPI_FLAG_IS_CLOCK_MODE_0 /*! Clock goes low while nCS is released, else high*/ = 0,
	QSPI_FLAG_IS_CLOCK_MODE_3 /*! Clock goes high while nCS is released, else low*/ = (1<<1),
	QSPI_FLAG_IS_FLASH_ID_2 /*! use flash 2, else 1*/= (1<<2),
	QSPI_FLAG_IS_ADDRESS_8_BITS /*! 8-bit address, default 32*/= (1<<3),
	QSPI_FLAG_IS_ADDRESS_16_BITS /*! 16-bit address, default 32*/ = (1<<4),
	QSPI_FLAG_IS_ADDRESS_24_BITS /*! 24-bit address, default 32*/ = (1<<5),
	QSPI_FLAG_IS_ADDRESS_32_BITS /*! 24-bit address, default 32*/ = (1<<6),
	QSPI_FLAG_IS_OPCODE_WRITE /*! Write an opcode */ = (1<<7),
	QSPI_FLAG_IS_OPCODE_DUAL /*! Use 2 lines for commands (default is 1 line) */ = (1<<8),
	QSPI_FLAG_IS_OPCODE_QUAD /*! Use 4 lines for commands (default is 1 line) */ = (1<<9),
	QSPI_FLAG_IS_DATA_DUAL /*! Use 2 lines for data */ = (1<<10),
	QSPI_FLAG_IS_DATA_QUAD /*! Use 4 lines for data */ = (1<<11),
	QSPI_FLAG_EXECUTE_COMMAND /*! Use with qspi_command_t in qspi_attr_t */ = (1<<12),
	QSPI_FLAG_IS_DATA_READ /*! Command reads data from device */ = (1<<13),
	QSPI_FLAG_IS_DATA_WRITE /*! Command write data to device */ = (1<<14),
	QSPI_FLAG_IS_ADDRESS_WRITE /*! Write the address as part of the command */ = (1<<15),
	QSPI_FLAG_IS_ADDRESS_DUAL /*! Use 2 lines for address (default is 1 line) */ = (1<<16),
	QSPI_FLAG_IS_ADDRESS_QUAD /*! Use 4 lines for address (default is 1 line) */ = (1<<17),
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

typedef struct MCU_PACK {
	u32 o_flags /*! Flags that affect the command */;
	u16 opcode /*! The opcode to send to the device */;
	u16 dummy_cycles /*! The number of dummy cycles to send */;
	u32 address /*! If an address is used in the command it is assigned here. */;
	u32 data_size /*! The number of data bytes to read or write */;
	u8 data[32] /*! The location of the data that is read or written */;
} qspi_command_t;

/*! \brief QSPI Set Attributes Data
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Flag bitmask */;
	qspi_pin_assignment_t pin_assignment /*! Pin Assignment */;
	u32 freq /*! Target operating frequency */;
	u32 resd[8];/*! */
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

#define I_QSPI_EXECCOMMAND _IOCTLRW(QSPI_IOC_IDENT_CHAR, I_MCU_TOTAL, qspi_command_t)

#define I_QSPI_TOTAL 1

#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_QSPI_H_ */

/*! @} */
