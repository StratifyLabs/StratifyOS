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

/*! \addtogroup FLASH Flash Memory
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The flash memory software module allows the user code to read memory
 * in the flash program space.  Reading the flash is portable across all supported MCU's.  However,
 * writing the flash is dependent on the MCU page size.
 *
 * This interface is typically only used with the bootloader.  The preferred way to access
 * the flash is using the \ref MEM interface.
 *
 *
 */

/*! \file
 * \brief Flash Memory Header File
 *
 */

#ifndef SOS_DEV_FLASH_H_
#define SOS_DEV_FLASH_H_


#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_VERSION (0x030000)
#define FLASH_IOC_IDENT_CHAR 'f'

#ifndef FLASH_MIN_WRITE_SIZE
/*! \brief See details below
 * \details This defines the MCU specific value for the
 * minimum write size.  This is also the minimum step
 * and alignment size for writing the flash.
 * \hideinitializer
 */
#define FLASH_MIN_WRITE_SIZE 256
#endif

#ifndef FLASH_MAX_WRITE_SIZE
/*! \brief See details below
 * \details This defines the MCU specific value for the
 * maximum write size for a flash operation.
 * \hideinitializer
 */
#define FLASH_MAX_WRITE_SIZE 1024
#endif


typedef struct {
	u32 o_flags;
	u32 o_events;
	u32 resd[8];
} flash_info_t;

typedef struct {
	u32 o_flags;
	u32 resd[8];
} flash_attr_t;

#define I_FLASH_GETVERSION _IOCTL(FLASH_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_FLASH_GETINFO _IOCTLR(FLASH_IOC_IDENT_CHAR, I_MCU_GETINFO, flash_info_t)
#define I_FLASH_SETATTR _IOCTLW(FLASH_IOC_IDENT_CHAR, I_MCU_SETATTR, flash_attr_t)
#define I_FLASH_SETACTION _IOCTLW(FLASH_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below
 * \details This request erases the page containing the
 * specified address.  The address is specified as
 * the ctl argument.
 *
 * \code
 * ioctl(flash_fd, I_FLASH_ERASEADDR, (void*)0x00001000 );
 * \endcode
 * \hideinitializer
 */
#define I_FLASH_ERASEADDR _IOCTL(FLASH_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)
#define I_FLASH_ERASE_ADDR I_FLASH_ERASEADDR

/*! \brief See details below
 * \details This request erases the page number specified by the ctl argument.
 *
 * Example:
 * \code
 * ioctl(flash_fd, I_FLASH_ERASEPAGE, 3); //erase page 3
 * \endcode
 * \hideinitializer
 */
#define I_FLASH_ERASEPAGE _IOCTL(FLASH_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)
#define I_FLASH_ERASE_PAGE I_FLASH_ERASEPAGE

/*! \brief See details below
 * \details This request gets the page number for the specified
 * address.  The ctl argument is the address and the return
 * value is the page.
 *
 * Example:
 * \code
 * int page;
 * page = ioctl(flash_fd, I_FLASH_GETPAGE, (void*)0x00001000 );
 * \endcode
 * \hideinitializer
 */
#define I_FLASH_GETPAGE _IOCTL(FLASH_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)
#define I_FLASH_GET_PAGE I_FLASH_GETPAGE

/*! \brief See details below
 * \details This request gets the page number for the specified
 * address.  The ctl argument is the address and the return
 * value is the page.
 *
 * Example:
 * \code
 * int page;
 * size = ioctl(flash_fd, I_FLASH_GET_PAGE);
 * \endcode
 * \hideinitializer
 */
#define I_FLASH_GETSIZE _IOCTL(FLASH_IOC_IDENT_CHAR, I_MCU_TOTAL + 3)
#define I_FLASH_GET_SIZE I_FLASH_GETSIZE

/*! \brief Flash page info for use with I_FLASH_GETPAGEINFO
 * \details This data structure holds the details of a flash page.
 */
typedef struct {
	u32 page /*! \brief The page number */;
	u32 addr /*! \brief The starting address */;
	u32 size /*! \brief The page size */;
} flash_pageinfo_t;

/*! \brief Flash page write data using I_FLASH_WRITEPAGE
 * \details This structure is used to write data to the flash
 * using I_FLASH_WRITEPAGE.
 */
typedef struct MCU_PACK {
	u32 addr /*! The address to write to */;
	u32 nbyte /*! The number of bytes to write */;
	u8 buf[FLASH_MAX_WRITE_SIZE] /*! \brief A buffer for writing to the flash */;
} flash_writepage_t;


/*!  \brief See details below
 * \details This request gets the page number for the specified
 * address.  The ctl argument is the address and the return
 * value is the page.
 *
 * Example:
 * \code
 * flash_pageinfo_t pageinfo;
 * page = ioctl(flash_fd, I_FLASH_GETPAGEINFO, &pageinfo );
 * \endcode
 * \hideinitializer
 */
#define I_FLASH_GETPAGEINFO _IOCTLRW(FLASH_IOC_IDENT_CHAR, I_MCU_TOTAL + 4, flash_pageinfo_t)
#define I_FLASH_GET_PAGEINFO I_FLASH_GETPAGEINFO

/*! \brief See details below
 * \details This request is used to write a page to the flash memory.
 * \code
 * flash_writepage_t pwrite;
 * pwrite.addr = 0x1000; //this should be on a boundary for the minimum page size
 * pwrite.nbyte = 256;  //This has to be a valid page write value for the architecture
 * memcpy(pwrite.buf, my_data, pwrite.nbyte);
 * ioctl(flash_fd, I_FLASH_WRITEPAGE, &pwrite);
 * \endcode
 */
#define I_FLASH_WRITEPAGE _IOCTLW(FLASH_IOC_IDENT_CHAR, I_MCU_TOTAL + 5, flash_writepage_t)

#define I_FLASH_TOTAL 6

#ifdef __cplusplus
}
#endif


#endif // SOS_DEV_FLASH_H_

/*! @} */


