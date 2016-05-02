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

/*! \addtogroup DISK Disk
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing disks.  The disk can be any external storage device such
 * as an SD card, serial flash chip, etc.  Disks are always read/write block devices.
 *
 *
 */

/*! \file  */

#ifndef DEV_DISK_H_
#define DEV_DISK_H_


#include <stdint.h>
#include "mcu/mcu.h"
#include "iface/dev/ioctl.h"

#define DISK_IOC_CHAR 'd'

typedef struct MCU_PACK {
	uint32_t start /*! \brief First block to erase */;
	uint32_t end /*! \brief Last block to erase */;
} disk_erase_block_t;

/*! \brief Disk attributes
 */
typedef struct MCU_PACK {
	uint16_t address_size /*! \brief Number of bytes per address location (1 for small devices 512 for larger ones */;
	uint16_t write_block_size /*! \brief Minimum write block size */;
	uint32_t num_write_blocks /*! \brief Number of write blocks (size is num_write_blocks*write_block_size */;
	uint32_t erase_block_size /*! \brief Minimum eraseable block size */;
	uint32_t erase_block_time /*! \brief Time in microseconds to erase one block */;
	uint32_t erase_device_time /*! \brief Time in microseconds to erase the device */;
	uint32_t bitrate /*! \brief Max bitrate */;
} disk_attr_t;


/*! \details This request applies the software write protect
 * to the entire device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DISK_PROTECT);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_PROTECT _IOCTL(DISK_IOC_CHAR, 0)

/*! \details This request removes the software write protect
 * from the entire device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DISK_UNPROTECT);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_UNPROTECT _IOCTL(DISK_IOC_CHAR, 1)

/*! \details This request erases a block of memory.  The ctl argurment
 * is an address that is contained in the block to erase.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DISK_ERASEBLOCK, 0x1000);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_ERASEBLOCK _IOCTL(DISK_IOC_CHAR, 2)
#define I_DISK_ERASE_BLOCK I_DISK_ERASEBLOCK

/*! \details This request erases the device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DISK_ERASEDEVICE);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_ERASEDEVICE _IOCTL(DISK_IOC_CHAR, 3)
#define I_DISK_ERASE_DEVICE I_DISK_ERASEDEVICE

/*! \details This request powers down the device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DISK_POWERDOWN);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_POWERDOWN _IOCTL(DISK_IOC_CHAR, 4)
#define I_DISK_POWER_DOWN I_DISK_POWERDOWN

/*! \details This request powers up the device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DISK_POWER_UP);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_POWERUP _IOCTL(DISK_IOC_CHAR, 5)
#define I_DISK_POWER_UP I_DISK_POWERUP

/*! \details This request gets the size of the memory on the device.
 *
 * Example:
 * \code
 * uint32_t size;
 * size = ioctl(fildes, I_DISK_GETSIZE);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_GETSIZE _IOCTL(DISK_IOC_CHAR, 6)
#define I_DISK_GET_SIZE I_DISK_GETSIZE

/*! \details This request gets the size of the smallest eraseable block.
 *
 * Example:
 * \code
 * uint32_t size;
 * size = ioctl(fildes, I_DISK_GETBLOCKSIZE);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_GETBLOCKSIZE _IOCTL(DISK_IOC_CHAR, 7)
#define I_DISK_GET_BLOCKSIZE I_DISK_GETBLOCKSIZE

/*! \details This request gets the amount of time to erase a block.
 *
 * Example:
 * \code
 * uint32_t useconds;
 * useconds = ioctl(fildes, I_DISK_GETBLOCKERASETIME);
 * ioctl(fildes, I_DISK_ERASEBLOCK, 0x1000);
 * usleep(useconds);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_GETBLOCKERASETIME _IOCTL(DISK_IOC_CHAR, 8)
#define I_DISK_GET_BLOCK_ERASETIME I_DISK_GETBLOCKERASETIME

/*! \details This request gets the amount of time required to erase the device.
 *
 * Example:
 * \code
 * uint32_t useconds;
 * useconds = ioctl(fildes, I_DISK_GETDEVICEERASETIME);
 * ioctl(fildes, I_DISK_ERASEDEVICE, 0x1000);
 * usleep(useconds);
 * \endcode
 * \hideinitializer
 */
#define I_DISK_GETDEVICEERASETIME _IOCTL(DISK_IOC_CHAR, 9)
#define I_DISK_GET_DEVICE_ERASETIME I_DISK_GETDEVICEERASETIME


/*! \details This request erases the blocks specified
 * in disk_erase_block_t
 *
 * \hideinitializer
 */
#define I_DISK_ERASEBLOCKS _IOCTLW(DISK_IOC_CHAR, 10, disk_erase_block_t)


/*! \details Request the disk attributes.
 * \hideinitializer
 */
#define I_DISK_GETATTR _IOCTLR(DISK_IOC_CHAR, 11, disk_attr_t)

/*! \details See if the disk is busy.
 * This ioctl call will return greater than one if the
 * device is busy and 0 if disk is not busy.
 *
 */
#define I_DISK_BUSY _IOCTL(DISK_IOC_CHAR, 12)


#define I_DISK_INIT _IOCTL(DISK_IOC_CHAR, 13)

#define I_DISK_TOTAL 14

#endif /* DEV_DISK_H_ */

/*! @} */
