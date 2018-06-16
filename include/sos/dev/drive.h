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

/*! \addtogroup DRIVE Drive
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

#define DRIVE_VERSION (0x030000)
#define DRIVE_IOC_IDENT_CHAR 'd'

enum {
    DRIVE_FLAG_PROTECT /*! Enables driver write protection. */ = (1<<0),
    DRIVE_FLAG_UNPROTECT /*! Disables driver write protection. */ = (1<<1),
    DRIVE_FLAG_ERASE_BLOCKS /*! Erases blocks on disk (use start and end to specify which blocks). */ = (1<<2),
    DRIVE_FLAG_ERASE_DEVICE /*! Erases the drive. */ = (1<<3),
    DRIVE_FLAG_POWERDOWN /*! Puts the drive in power down mode. */ = (1<<4),
    DRIVE_FLAG_POWERUP /*! Powers up the driver (after power down). */ = (1<<5),
    DRIVE_FLAG_INIT /*! Initializes the drive. */ = (1<<6)
} drive_flags_t;

/*! \brief Drive Info
 * \details This is the data structure for accessing
 * information about the device.
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Attribute flags supported by this driver */;
	u32 o_events /*! MCU Event flags supported by this driver */;
	u16 address_size /*! Number of bytes per address location (1 for small devices 512 for larger ones */;
	u16 write_block_size /*! Minimum write block size */;
	u32 num_write_blocks /*! Number of write blocks (size is num_write_blocks*write_block_size */;
	u32 erase_block_size /*! Minimum eraseable block size */;
	u32 erase_block_time /*! Time in microseconds to erase one block */;
	u32 erase_device_time /*! Time in microseconds to erase the device */;
	u32 bitrate /*! Max bitrate */;
	u32 resd[8];
} drive_info_t;


/*! \brief Drive Attributes
 * \details This is the data structure used with setting drive attributes.
 *
 */
typedef struct MCU_PACK {
    u32 o_flags /*! Drive flags such as \ref DRIVE_FLAG_INIT */;
    u32 start /*! Start block (used with \ref DRIVE_FLAG_ERASE_BLOCKS) */;
    u32 end /*! End block (used with \ref DRIVE_FLAG_ERASE_BLOCKS) */;
	u32 resd[8];
} drive_attr_t;

/*! \details Gets the driver version. */
#define I_DRIVE_GETVERSION _IOCTL(DRIVE_IOC_IDENT_CHAR, I_MCU_GETVERSION)
/*! \details Gets the drive info (drive_info_t). */
#define I_DRIVE_GETINFO _IOCTLR(DRIVE_IOC_IDENT_CHAR, I_MCU_GETINFO, drive_info_t)
/*! \details Sets the drive attributes (\sa drive_attr_t). */
#define I_DRIVE_SETATTR _IOCTLW(DRIVE_IOC_IDENT_CHAR, I_MCU_SETATTR, drive_attr_t)
#define I_DRIVE_SETACTION _IOCTLW(DRIVE_IOC_IDENT_CHAR, I_MCU_SETATTR, mcu_action_t)

/*! \details See if the drive is busy.
 * This ioctl call will return greater than one if the
 * device is busy and 0 if drive is not busy.
 *
 */
#define I_DRIVE_ISBUSY _IOCTL(DRIVE_IOC_IDENT_CHAR, I_MCU_TOTAL)


#define I_DRIVE_TOTAL 1

#endif /* SOS_DEV_DRIVE_H_ */

/*! @} */
