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

/*! \addtogroup BOOTLOADER_DEV Bootloader Access
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Bootloader provides the IOCTL definitions for accessing the bootloader.  This interface
 * is not available on any device when Stratify OS is running.  It is only available
 * to the bootloader.
 *
 */

/*! \file
 * \brief Analog to Digital Converter Header File
 *
 */

#ifndef SOS_DEV_BOOTLOADER_H_
#define SOS_DEV_BOOTLOADER_H_


#include "mcu/types.h"
#include "sos/link/transport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOOTLOADER_VERSION (0x030000)


#define BOOTLOADER_IOC_IDENT_CHAR 'b'


/*! \brief See below for details
 * \details This specifies where in the image the hardware
 * ID value resides.
 */
#define BOOTLOADER_HARDWARE_ID_OFFSET (28)
#define BOOTLOADER_API_ADDR (36)


/*! \brief Bootloader attributes.
 * \details This structure contains the attributes for the bootloader.
 */
typedef struct MCU_PACK {
	u32 version /*! \brief The bootloader version */;
	u32 serialno[4] /*! \brief The device serial number */;
	u32 startaddr /*! \brief The start address of the target program (read-only) */;
	u32 hardware_id /*! \brief The hardware ID of the bootloader (must match image) */;
} bootloader_info_t;

typedef bootloader_info_t bootloader_attr_t;

typedef struct MCU_PACK {
    uint32_t version;
    uint32_t serialno[4];
    uint32_t startaddr;
} bootloader_attr_legacy_t;


/*! \brief This is the size of a bootloader flash page.
 */
#define BOOTLOADER_WRITEPAGESIZE 1024

/*! \brief See details below.
 * \details This structure is used for writing
 * data to the bootloader flash using \ref I_BOOTLOADER_WRITEPAGE.
 */
typedef struct MCU_PACK {
	u32 addr /*! \brief The address to write to */;
	u32 nbyte /*! \brief The number of bytes to write */;
	u8 buf[BOOTLOADER_WRITEPAGESIZE] /*! \brief A buffer for writing to the flash */;
} bootloader_writepage_t;



/*! \brief See below for details.
 * \details This request sets the functionality of the specified pin.
 *
 */
#define I_BOOTLOADER_ERASE _IOCTL(BOOTLOADER_IOC_IDENT_CHAR, 0)

/*! \brief See below for details.
 * \details This request loads the bootloader attributes.
 *
 * \code
 * bootloader_attr_t attr;
 * link_ioctl(LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETINFO, &info);
 * \endcode
 */
#define I_BOOTLOADER_GETINFO _IOCTLR(BOOTLOADER_IOC_IDENT_CHAR, 1, bootloader_info_t)
#define I_BOOTLOADER_ATTR I_BOOTLOADER_GETINFO

#define I_BOOTLOADER_GETATTR_LEGACY _IOCTLR(BOOTLOADER_IOC_IDENT_CHAR, 1, bootloader_attr_legacy_t)


/*! \brief See below for details.
 * \details This request invokes a reset.  The third IOCTL argument is a
 * boolean value saying whether or not to start the bootloader.  If this value
 * is false, the application is loaded.
 *
 * \code
 * bool invoke_bootloader = false;
 * link_ioctl(LINK_BOOTLOADER_FILDES, I_BOOTLOADER_RESET, NULL);
 * \endcode
 */
#define I_BOOTLOADER_RESET _IOCTL(BOOTLOADER_IOC_IDENT_CHAR, 2)

/*! \brief See below for details.
 * \details This request writes a page to the flash memory.
 *
 * \code
 * bootloader_writepage_t attr;
 * attr.loc = 0x1000;
 * attr.nbyte = 1024;
 * memset(attr.buf, 0xFF, FLASH_MAX_WRITE_SIZE);
 *  //copy data in to attr.buf up to FLASH_MAX_WRITE_SIZE bytes
 * ioctl(fd, I_BOOTLOADER_WRITEPAGE, &attr);
 * \endcode
 */
#define I_BOOTLOADER_WRITEPAGE _IOCTLW(BOOTLOADER_IOC_IDENT_CHAR, 3, bootloader_writepage_t)


#define I_BOOTLOADER_TOTAL 4

#ifdef __cplusplus
}
#endif


#endif // SOS_DEV_BOOTLOADER_H_

/*! @} */

