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

/*! \addtogroup DRIVE Disk
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing drives.  The drive can be any external storage device such
 * as an SD card, serial flash chip, etc.  Disks are always read/write block devices.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_DRIVE_H_
#define SOS_DEV_DRIVE_H_

#include "mcu/types.h"

#define DRIVE_IOC_CHAR 'd'

typedef struct MCU_PACK {
	uint32_t start /*! \brief First block to erase */;
	uint32_t end /*! \brief Last block to erase */;
} drive_erase_block_t;

/*! \brief Disk attributes
 */
typedef struct MCU_PACK {
	u16 address_size /*! \brief Number of bytes per address location (1 for small devices 512 for larger ones */;
	u16 write_block_size /*! \brief Minimum write block size */;
	u32 num_write_blocks /*! \brief Number of write blocks (size is num_write_blocks*write_block_size */;
	u32 erase_block_size /*! \brief Minimum eraseable block size */;
	u32 erase_block_time /*! \brief Time in microseconds to erase one block */;
	u32 erase_device_time /*! \brief Time in microseconds to erase the device */;
	u32 bitrate /*! \brief Max bitrate */;
} drive_info_t;


/*! \details This request applies the software write protect
 * to the entire device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DRIVE_PROTECT);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_PROTECT _IOCTL(DRIVE_IOC_CHAR, 0)

/*! \details This request removes the software write protect
 * from the entire device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DRIVE_UNPROTECT);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_UNPROTECT _IOCTL(DRIVE_IOC_CHAR, 1)

/*! \details This request erases a block of memory.  The ctl argurment
 * is an address that is contained in the block to erase.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DRIVE_ERASEBLOCK, 0x1000);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_ERASEBLOCK _IOCTL(DRIVE_IOC_CHAR, 2)
#define I_DRIVE_ERASE_BLOCK I_DRIVE_ERASEBLOCK

/*! \details This request erases the device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DRIVE_ERASEDEVICE);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_ERASEDEVICE _IOCTL(DRIVE_IOC_CHAR, 3)
#define I_DRIVE_ERASE_DEVICE I_DRIVE_ERASEDEVICE

/*! \details This request powers down the device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DRIVE_POWERDOWN);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_POWERDOWN _IOCTL(DRIVE_IOC_CHAR, 4)
#define I_DRIVE_POWER_DOWN I_DRIVE_POWERDOWN

/*! \details This request powers up the device.
 *
 * Example:
 * \code
 * ioctl(fildes, I_DRIVE_POWER_UP);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_POWERUP _IOCTL(DRIVE_IOC_CHAR, 5)
#define I_DRIVE_POWER_UP I_DRIVE_POWERUP

/*! \details This request gets the size of the memory on the device.
 *
 * Example:
 * \code
 * uint32_t size;
 * size = ioctl(fildes, I_DRIVE_GETSIZE);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_GETSIZE _IOCTL(DRIVE_IOC_CHAR, 6)
#define I_DRIVE_GET_SIZE I_DRIVE_GETSIZE

/*! \details This request gets the size of the smallest eraseable block.
 *
 * Example:
 * \code
 * uint32_t size;
 * size = ioctl(fildes, I_DRIVE_GETBLOCKSIZE);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_GETBLOCKSIZE _IOCTL(DRIVE_IOC_CHAR, 7)
#define I_DRIVE_GET_BLOCKSIZE I_DRIVE_GETBLOCKSIZE

/*! \details This request gets the amount of time to erase a block.
 *
 * Example:
 * \code
 * uint32_t useconds;
 * useconds = ioctl(fildes, I_DRIVE_GETBLOCKERASETIME);
 * ioctl(fildes, I_DRIVE_ERASEBLOCK, 0x1000);
 * usleep(useconds);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_GETBLOCKERASETIME _IOCTL(DRIVE_IOC_CHAR, 8)
#define I_DRIVE_GET_BLOCK_ERASETIME I_DRIVE_GETBLOCKERASETIME

/*! \details This request gets the amount of time required to erase the device.
 *
 * Example:
 * \code
 * uint32_t useconds;
 * useconds = ioctl(fildes, I_DRIVE_GETDEVICEERASETIME);
 * ioctl(fildes, I_DRIVE_ERASEDEVICE, 0x1000);
 * usleep(useconds);
 * \endcode
 * \hideinitializer
 */
#define I_DRIVE_GETDEVICEERASETIME _IOCTL(DRIVE_IOC_CHAR, 9)
#define I_DRIVE_GET_DEVICE_ERASETIME I_DRIVE_GETDEVICEERASETIME


/*! \details This request erases the blocks specified
 * in drive_erase_block_t
 *
 * \hideinitializer
 */
#define I_DRIVE_ERASEBLOCKS _IOCTLW(DRIVE_IOC_CHAR, 10, drive_erase_block_t)


/*! \details Request the drive attributes.
 * \hideinitializer
 */
#define I_DRIVE_GETINFO _IOCTLR(DRIVE_IOC_CHAR, 11, drive_info_t)

/*! \details See if the drive is busy.
 * This ioctl call will return greater than one if the
 * device is busy and 0 if drive is not busy.
 *
 */
#define I_DRIVE_BUSY _IOCTL(DRIVE_IOC_CHAR, 12)


#define I_DRIVE_INIT _IOCTL(DRIVE_IOC_CHAR, 13)

#define I_DRIVE_TOTAL 14

#endif /* SOS_DEV_DRIVE_H_ */

/*! @} */
