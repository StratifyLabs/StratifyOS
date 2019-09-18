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

/*! \addtogroup SYS_FIFO FIFO Buffer
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device is a FIFO RAM buffer.  A write to the buffer adds
 * data to the FIFO.  A read from the buffer reads from the FIFO.  If the FIFO
 * fills up, additional writes will block until data arrives.  Similarly, if the FIFO
 * is empty, a read will block until data arrives.
 *
 */

/*! \file  */

#ifndef SOS_DEV_FIFO_H_
#define SOS_DEV_FIFO_H_

#include "mcu/types.h"

#define FIFO_VERSION (0x030000)
#define FIFO_IOC_CHAR 'f'

enum {
	FIFO_FLAG_NONE = 0,
	FIFO_FLAG_SET_WRITEBLOCK /*! Write block flag */ = (1<<0),
	FIFO_FLAG_IS_OVERFLOW /*! Overflow flag */ = (1<<1),
	FIFO_FLAG_NOTIFY_WRITE /*! Notify on write */ = (1<<2),
	FIFO_FLAG_NOTIFY_READ /*! Notify on read */ = (1<<3),
	FIFO_FLAG_INIT /*! Initialize the FIFO */ = (1<<4),
	FIFO_FLAG_EXIT /*! Shutdown the FIFO */ = (1<<5),
	FIFO_FLAG_FLUSH /*! Flush the FIFO */ = (1<<6),
	FIFO_FLAG_IS_READ_BUSY /*! Set internally when FIFO is reading */ = (1<<7),
	FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY /*! Set internally when FIFO is written while reading */ = (1<<8),
	FIFO_FLAG_IS_WRITE_BUSY /*! Set internally when FIFO is being written */ = (1<<9),
	FIFO_FLAG_IS_WRITE_WHILE_WRITE_BUSY /*! Set internally when FIFO is written while being written */ = (1<<10)
};

typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask to indicate supported features */;
	u32 size /*! The total number of bytes in the FIFO */;
	u32 size_ready /*! The number of bytes ready to be read from the FIFO */;
	u32 overflow /*! If non-zero, indicates data received in the buffer was discarded */;
	u32 resd[8];
} fifo_info_t;

/*! \brief FIFO Attributes
 * \details This structure defines the attributes of a FIFO.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Use to set the write block */;
	u32 resd[8];
} fifo_attr_t;

/*! \brief FIFO Peek data */
/*! \details This is used when peeking at data in the FIFO */
typedef struct MCU_PACK {
	u32 loc /*! The location to peek */;
	char * buf /*! A pointer to the buffer to receive peek bytes */;
	int nbyte /*! The number of bytes to peek */;
} fifo_peek_t;

#define I_FIFO_GETVERSION _IOCTL(FIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief See below.
 * \details This request gets the size of the FIFO in bytes.
 * Example:
 * \code
 * fifo_attr_t attr;
 * ioctl(fifo_fd, I_FIFO_ATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_FIFO_GETINFO _IOCTLR(FIFO_IOC_CHAR, 0, fifo_info_t)

#define I_FIFO_SETATTR _IOCTLW(FIFO_IOC_CHAR, 1, fifo_attr_t)


/*! \brief This request flushes all data out of the fifo (third ioctl() arg is omitted).
 */
#define I_FIFO_FLUSH _IOCTL(FIFO_IOC_CHAR, 2)

/*! \brief This request initializes the fifo (third ioctl() arg is omitted).
 */
#define I_FIFO_INIT _IOCTL(FIFO_IOC_CHAR, 3)

/*! \brief This request shuts down the operation of the FIFO.
 */
#define I_FIFO_EXIT _IOCTL(FIFO_IOC_CHAR, 4)


/*! \brief Request a peek at the data in the FIFO.
 * \details This request allows a process to peek
 * at the data in the FIFO without reading it.
 *
 * \code
 * char buffer[16];
 * fifo_peek_t peek_req;
 * peek_req.buf = buffer; //Destination buffer
 * peek_req.loc = 0; //first available bytes in FIFO
 * peek_req.nbyte = 16; //peek up to 16 bytes
 * ioctl(fifo_fd, I_FIFO_PEEK, &peek);
 * \endcode
 *
 */
#define I_FIFO_PEEK _IOCTLR(FIFO_IOC_CHAR, 5, fifo_peek_t)

#define I_FIFO_TOTAL 6


#endif /* SOS_DEV_FIFO_H_ */


/*! @} */
