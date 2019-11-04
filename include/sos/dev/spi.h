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

/*! \addtogroup SPI Serial Peripheral Interface (SPI)
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
 * The following is an example of how to read/write the SPI from an application:
 *
 * \code
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <dev/sos/spi.h>
 *
 * int fd;
 * char buffer[32];
 * spi_attr_t attr;
 * fd = open("/dev/spi0", O_RDWR);
 *
 * attr.o_flags = SPI_FLAG_SET_MASTER | SPI_FLAG_IS_MODE0 | SPI_FLAG_IS_FORMAT_SPI;
 * attr.width = 8;
 * attr.freq = 1000000;
 * attr.pin_assignment.mosi = mcu_pin(0,0);
 * attr.pin_assignment.miso = mcu_pin(0,1);
 * attr.pin_assignment.sck = mcu_pin(0,2);
 * attr.pin_assignment.cs = mcu_pin(0xff,0xff); //only used by slave
 * ioctl(fd, I_SPI_SETATTR, &attr);
 *
 * read(fd, buffer, 32);
 * write(fd, buffer, 32);
 *
 * close(fd);
 * \endcode
 *
 * For full duplex operation use:
 *
 * \code
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <dev/sos/spi.h>
 *
 * int fd;
 * spi_attr_t attr;
 * fd = open("/dev/spi0", O_RDWR);
 * char read_buffer[32];
 * char write_buffer[32];
 * struct aiocb aio;
 *
 *
 * attr.o_flags = SPI_FLAG_SET_MASTER | SPI_FLAG_IS_MODE0 | SPI_FLAG_IS_FORMAT_SPI | SPI_FLAG_IS_FULL_DUPLEX;
 * attr.width = 8;
 * attr.freq = 1000000;
 * attr.pin_assignment.mosi = mcu_pin(0,0);
 * attr.pin_assignment.miso = mcu_pin(0,1);
 * attr.pin_assignment.sck = mcu_pin(0,2);
 * attr.pin_assignment.cs = mcu_pin(0xff,0xff); //only used by slave
 * ioctl(fd, I_SPI_SETATTR, &attr);
 *
 * aio.aio_buf = read_buffer;
 * aio.aio_nbytes = 32;
 * aio.aio_fildes = fd;
 *
 * aio_read(fd, &aio); //this won't do anything on the bus
 * write(fd, write_buffer, 32); //this will read to read_buffer and write from write_buffer
 *
 * printf("AIO return is %d\n", aio_return(&aio)); //AIO completes when the write completes
 *
 * close(fd);
 * \endcode
 *
 *
 */

/*! \file
 * \brief Serial Peripheral Interface Header File
 *
 */

#ifndef SOS_DEV_SPI_H_
#define SOS_DEV_SPI_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SPI_VERSION (0x030000)
#define SPI_IOC_IDENT_CHAR 's'

/*! \details SPI flags used with
 * spi_attr_t.o_flags and I_SPI_SETATTR.
 *
 */
typedef enum {
	SPI_FLAG_IS_FORMAT_SPI /*! SPI Format */ = (1<<0),
	SPI_FLAG_IS_FORMAT_TI /*! TI Format */ = (1<<1),
	SPI_FLAG_IS_FORMAT_MICROWIRE /*! Microwire format */ = (1<<2),
	SPI_FLAG_IS_MODE0 /*! SPI Mode 0 */ = (1<<3),
	SPI_FLAG_IS_MODE1 /*! SPI Mode 1 */ = (1<<4),
	SPI_FLAG_IS_MODE2 /*! SPI Mode 2 */ = (1<<5),
	SPI_FLAG_IS_MODE3 /*! SPI Mode 3 */ = (1<<6),
	SPI_FLAG_SET_MASTER /*! SPI Master */ = (1<<7),
	SPI_FLAG_SET_SLAVE /*! SPI Slave */ = (1<<8),
	SPI_FLAG_IS_FULL_DUPLEX /* Use SPI_FLAG_SET_FULL_DUPLEX */ = (1<<9),
	SPI_FLAG_SET_FULL_DUPLEX /*! Set Full duplex mode but don't change any other settings */ = (1<<9),
	SPI_FLAG_IS_HALF_DUPLEX /* Use SPI_FLAG_SET_HALF_DUPLEX */ = (1<<10),
	SPI_FLAG_SET_HALF_DUPLEX /*! Set Half duplex mode but don't change any other settings */ = (1<<10),
	SPI_FLAG_IS_LSB_FIRST /*! LSB first (non-standard) */ = (1<<11),
} spi_flag_t;

typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask of supported flags */;
	u32 o_events /*! Bitmask of supported events */;
	u32 resd[8];
} spi_info_t;

/*! \brief SPI Pin Assignment Data
 *
 */
typedef struct MCU_PACK {
	mcu_pin_t miso /*! MISO pin */;
	mcu_pin_t mosi /*! MOSI pin */;
	mcu_pin_t sck /*! Serial clock pin */;
	mcu_pin_t cs /*! Chip select pin */;
} spi_pin_assignment_t;


/*! \brief SPI Set Attributes Data
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Flag bitmask */;
	spi_pin_assignment_t pin_assignment /*! Pin Assignment */;
	u32 freq /*! Target operating frequency */;
	u8 width /*! Width for transactions */;
	u32 resd[8];
} spi_attr_t;

#define I_SPI_GETVERSION _IOCTL(SPI_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief This request gets the SPI attributes.
 * \hideinitializer
 */
#define I_SPI_GETINFO _IOCTLR(SPI_IOC_IDENT_CHAR, I_MCU_GETINFO, spi_info_t)

/*! \brief This request sets the SPI attributes.
 * \hideinitializer
 */
#define I_SPI_SETATTR _IOCTLW(SPI_IOC_IDENT_CHAR, I_MCU_SETATTR, spi_attr_t)
#define I_SPI_SETACTION _IOCTLW(SPI_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


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
