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
    QSPI_FLAG_READ_MEM_MAPPED_MODE /*! Configure the QSPI as mem mapped mode(read only)*/ = (1<<1),
    QSPI_FLAG_CLK_HIGH_WHILE_CS /*!<Clk goes high while nCS is released, else low*/ = (1<<2),
    QSPI_FLAG_FLASH_ID_2 /*!<use flash 2, else 1*/= (1<<3),
    QSPI_FLAG_INSTRUCTION_1_LINE/*!<write command use one line instead 4*/ = (1<<7),
    QSPI_FLAG_INSTRUCTION_4_LINE/*!<write command use one line instead 4*/ = (1<<7),
    QSPI_FLAG_ADDRESS_8_BITS    /*!<8-bit address, default 32*/= (1<<8),
    QSPI_FLAG_ADDRESS_16_BITS   /*!<16-bit address, default 32*/ = (1<<9),
    QSPI_FLAG_ADDRESS_24_BITS   /*!<24-bit address, default 32*/ = (1<<10),
    QSPI_FLAG_ADDRESS_32_BITS   /*!<24-bit address, default 32*/ = (1<<10),
    QSPI_FLAG_ADDRESS_1_LINE /*!<use addressed command*/ = (1<<12),
    QSPI_FLAG_ADDRESS_4_LINES /*!<use addressed command*/ = (1<<12),
    QSPI_FLAG_DATA_1_LINE /*!<use addressed command*/ = (1<<13),
    QSPI_FLAG_DATA_4_LINES /*!<use addressed command*/ = (1<<13),
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
    u32 read_instruction;/*! use for set read operations */
    u32 mem_mapped_read_instruction; /*! use for read operations in mem mapped mode*/
    u32 write_instruction;/*! use for write operations */
    u32 dummy_cycle;/*! number dummy cycles for read operations*/
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

/*! \brief See details below.
 * \details This tells the qspi bus to
 * set a command fro data to bus.
 * The ctl argument to qspi_ioctl() is the
 * word to put (not a pointer to qspi_attr_t).
 *
 * The following example puts WRITE_ENABLE_CMD(write enable command)
 * \code
 *
 * ioctl(fd, I_QSPI_COMMAND, WRITE_ENABLE_CMD);
 * \endcode
 * \hideinitializer
 */
#define I_QSPI_COMMAND _IOCTL(QSPI_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)

typedef struct MCU_PACK {
    u32 command;    /*<! command type*/
    u32 address;    /*<! address will use at command*/
} qspi_addr_command_config_t;
typedef struct MCU_PACK {
    u32 command;    /*<! command type*/
    u32 regs_number;    /*<! regs number for read*/
    u8 * data;      /*<! read data */
} qspi_read_regs_config_t;
typedef struct MCU_PACK {
    u32 command;    /*<! command type*/
    u32 regs_number;    /*<! regs number for read*/
    u8 * data;      /*<! read data */
} qspi_write_regs_config_t;

#define I_QSPI_ADDR_COMMAND _IOCTL(QSPI_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)
#define I_QSPI_READ_REGS _IOCTL(QSPI_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)
#define I_QSPI_WRITE_REGS _IOCTL(QSPI_IOC_IDENT_CHAR, I_MCU_TOTAL + 3)
#define I_QSPI_TOTAL 4

#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_QSPI_H_ */

/*! @} */
