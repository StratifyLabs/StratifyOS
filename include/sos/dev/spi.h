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

/*! \addtogroup SPI Serial Peripheral Interface (SPI) Master
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details This module implements a hardware SPI master.  The chip select functionality
 * is not part of this module and must be implemented separately.  This allows the application
 * to use the SPI bus to access multiple devices.  More information about accessing peripheral IO
 * is in the \ref IFACE_DEV section.
 *
 *
 * The following is an example of how to read/write the SPI in an OS environment:
 *
 * \code
 *
 * \endcode
 *
 */

/*! \file
 * \brief Serial Peripheral Interface Header File
 *
 */

#ifndef SOS_DEV_SPI_H_
#define SOS_DEV_SPI_H_

#include <stdint.h>

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SPI_IOC_IDENT_CHAR 's'

typedef enum {
	SPI_FLAG_IS_FORMAT_SPI = (1<<0) /*! SPI Format */,
	SPI_FLAG_IS_FORMAT_TI = (1<<1) /*! TI Format */,
	SPI_FLAG_IS_FORMAT_MICROWIRE = (1<<2) /*! Microwire format */,
	SPI_FLAG_IS_MODE0 = (1<<3) /*! SPI Mode 0 */,
	SPI_FLAG_IS_MODE1 = (1<<4) /*! SPI Mode 1 */,
	SPI_FLAG_IS_MODE2 = (1<<5) /*! SPI Mode 2 */,
	SPI_FLAG_IS_MODE3 = (1<<6) /*! SPI Mode 3 */,
	SPI_FLAG_SET_MASTER = (1<<7) /*! SPI Master */,
	SPI_FLAG_SET_SLAVE = (1<<8) /*! SPI Slave */,
	SPI_FLAG_IS_FULL_DUPLEX = (1<<9) /*! Full duplex mode (data is written first then read into the same buffer) */,
	SPI_FLAG_IS_HALF_DUPLEX = (1<<10) /*! Half duplex mode (default mode) */,
} spi_flag_t;

typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask of supported flags */;
	u32 o_events /*! Bitmask of supported events */;
} spi_info_t;

#define SPI_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	mcu_pin_t miso;
	mcu_pin_t mosi;
	mcu_pin_t sck;
	mcu_pin_t cs;
} spi_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags;
	spi_pin_assignment_t pin_assignment;
	u32 freq;
	u8 width;
} spi_attr_t;


/*! \brief This request gets the SPI attributes.
 * \hideinitializer
 */
#define I_SPI_GETINFO _IOCTLR(SPI_IOC_IDENT_CHAR, I_MCU_GETINFO, spi_info_t)

/*! \brief This request sets the SPI attributes.
 * \hideinitializer
 */
#define I_SPI_SETATTR _IOCTLW(SPI_IOC_IDENT_CHAR, I_MCU_SETATTR, spi_attr_t)
#define I_SPI_SETACTION _IOCTLW(SPI_IOC_IDENT_CHAR, I_MCU_SETACTION, spi_action_t)


/*! \brief See details below.
 * \details This tells the SPI bus to
 * get a swap a byte from the bus.  spi_ioctl() returns the
 * word read from the bus;
 * The ctl argument to spi_ioctl() is the
 * word to put (not a pointer to spi_attr_t).
 *
 * The following example puts 0xAC on the bus and stores
 * whatever is read from the bus in swapped_word.
 * \code
 * int swapped_word;
 * swapped_word = ioctl(spi_fd, I_SPI_SWAP, 0xAC);
 * \endcode
 * \hideinitializer
 */
#define I_SPI_SWAP _IOCTL(SPI_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)


#define I_SPI_TOTAL 1

#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_SPI_H_ */

/*! @} */
