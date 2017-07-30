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

/*! \addtogroup EMC External Memory Controller (EMC)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The EMC API allows configuration of the external memory controller.
 *
 * The EMC is usually initialized by the OS at startup using the following sample code:
 *
 * \code
 *
 *
 * \endcode
 *
 */

/*! \file
 * \brief External Interrupt Header File
 *
 */


#ifndef SOS_DEV_EMC_H_
#define SOS_DEV_EMC_H_

#include "mcu/types.h"

#define EMC_IOC_IDENT_CHAR 'E'

/*! \brief See below for details */
/*! \details These are the flags for mode in emc_attr_t */
typedef enum {
	EMC_MODE_BUS8 /*! 8-bit bus width */ = (1<<0),
	EMC_MODE_BUS16 /*! 16-bit bus width */ = (1<<1),
	EMC_MODE_BUS32 /*! 32-bit bus width (default) */ = 0,
	EMC_MODE_STATIC /*! Static memory (default) */ = 0,
	EMC_MODE_DYNAMIC /*! Dynamic memory */ = (1<<2),
	EMC_MODE_PAGEMODE /*! Page mode */ = (1<<3),
	EMC_MODE_CSPOL_HIGH /*! Active high chip select */ = (1<<4),
	EMC_MODE_CSPOL_LOW /*! Active high chip select (default) */ = 0,
	EMC_MODE_BYTELANE_HIGH /*! Byte lane bytes are high for reads and low for writes (default) */ = 0,
	EMC_MODE_BYTELANE_LOW /*! Byte lane bytes are low for reads and writes */ = (1<<5),
	EMC_MODE_EXTWAIT /*! Enable Extended wait */ = (1<<6),
	EMC_MODE_BUFFER /*! Enable buffer */ = (1<<7),
	EMC_MODE_WP /*! Enable write protect */ = (1<<8),
	EMC_MODE_LITTLE_ENDIAN /*! Little Endian mode (default) */ = 0,
	EMC_MODE_BIT_ENDIAN /*! Little Endian mode (default) */ = (1<<9),
	EMC_MODE_ADDR_MIRROR /*! Address mirror */ = (1<<10),
	EMC_MODE_USEBLS /*! Enable BLS signal */ = (1<<11)
} emc_mode_t;

typedef struct MCU_PACK {
	uint8_t port /*! EMC port */;
	uint8_t channel /*! EMC Channel (chip select line) */;
	uint8_t pin_assign /*! EMC Pin assignment */;
	uint8_t wait_write_enable /*! Clock delay from chip select to write enable */;
	uint8_t wait_output_enable /*! Clock delay from chip select to output enable */;
	uint8_t wait_rd /*! Read mode delay */;
	uint8_t wait_page /*! Number of wait states for async page mode read */;
	uint8_t wait_wr /*! Write mode delay (SRAM) */;
	uint8_t wait_turn /*! Turn around cycles */;
	uint8_t addr_width /*! The width of the address bus */;
	uint32_t mode /*! Mode flags */;
	uint32_t size /*! Size of the external memory */;
	uint32_t clock /*! Maximum clock rate */;
} emc_attr_t;


typedef u32 emc_info_t;


#define I_EMC_GETINFO _IOCTLR(EMC_IOC_IDENT_CHAR, I_MCU_GETINFO, eint_info_t)

/*! \brief IOCTL request to set the attributes
 */
#define I_EMC_SETATTR _IOCTLW(EMC_IOC_IDENT_CHAR, I_MCU_SETATTR, eint_attr_t)

#define I_EMC_SETACTION _IOCTLW(EMC_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

#define I_EMC_TOTAL 0


#endif /* SOS_DEV_EMC_H_ */

/*! @} */
