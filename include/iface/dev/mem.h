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

/*! \addtogroup MEM Memory (RAM/Flash)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The flash memory software module allows the user code to read memory
 * in the flash program space.  Reading the flash is portable across all supported MCU's.  However,
 * writing the flash is dependent on the MCU page size.
 */

/*! \file
 * \brief Flash Memory Header File
 *
 */

#ifndef IFACE_DEV_MEM_H_
#define IFACE_DEV_MEM_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_IOC_IDENT_CHAR 'M'

#ifndef MEM_MIN_FLASH_WRITE_SIZE
/*! \brief This defines the MCU specific value for the
 * minimum write size.  This is also the minimum step
 * and alignment size for writing the flash.
 * \hideinitializer
 */
#define MEM_MIN_FLASH_WRITE_SIZE 256
#endif

#ifndef MEM_MAX_FLASH_WRITE_SIZE
/*! \brief This defines the MCU specific value for the
 * maximum write size for a flash operation.
 * \hideinitializer
 */
#define MEM_MAX_FLASH_WRITE_SIZE 1024
#endif


/*! \brief Holds the devices attributes.
 * \details This defines the attributes of the memory.
 *
 * It is used with \ref I_MEM_GETATTR.  The attributes
 * are read-only so I_MEM_SETATTR has no effect.
 *
 */
typedef struct MCU_PACK {
	u32 flash_pages /*! \brief The total number of flash pages */;
	u32 flash_size /*! \brief The total size of the flash memory */;
	u32 ram_pages /*! \brief The total number of RAM pages */;
	u32 ram_size /*! \brief The total size of the RAM */;
} mem_attr_t;

/*! \details This lists each type of page.
 *
 */
enum {
	MEM_PAGEINFO_TYPE_QUERY /*! Query the page type */,
	MEM_PAGEINFO_TYPE_RAM /*! RAM */,
	MEM_PAGEINFO_TYPE_FLASH /*! Flash */,
	MEM_PAGEINFO_TYPE_TOTAL
};

/*! \brief Holds the characteristics of a page.
 * \details This structure contains the details
 * for a page of memory.
 */
typedef struct MCU_PACK {
	int32_t num /*! \brief the page number */;
	uint8_t type /*! \brief RAM or FLASH (page numbers are not unique between types) */;
	u32 addr /*! \brief the address of the page */;
	u32 size /*! \brief the size of the page */;
} mem_pageinfo_t;


typedef struct MCU_PACK {
	u32 addr /*! The address to write to */;
	u32 nbyte /*! The number of bytes to write */;
	u8 buf[256] /*! \brief A buffer for writing to the flash */;
} mem_writepage_t;

/*! \brief This request gets the memory attributes.
 * \hideinitializer
 */
#define I_MEM_ATTR _IOCTLR(MEM_IOC_IDENT_CHAR, I_GLOBAL_ATTR, mem_attr_t)
#define I_MEM_GETATTR I_MEM_ATTR
#define I_MEM_SETATTR _IOCTLW(MEM_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, mem_attr_t)
#define I_MEM_SETACTION _IOCTLW(MEM_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, mem_attr_t)

/*! \brief See details below.
 * \details This request erases the page number specified by the ctl argument.
 *
 * Example:
 * \code
 * ioctl(mem_fd, I_MEM_ERASEPAGE, 3); //erase page 3
 * \endcode
 * \hideinitializer
 *
 * This function has no effect for pages that are not flash pages.
 *
 * \hideinitializer
 */
#define I_MEM_ERASEPAGE _IOCTL(MEM_IOC_IDENT_CHAR, I_GLOBAL_TOTAL)
#define I_MEM_ERASE_PAGE I_MEM_ERASEPAGE

/*! \brief See details below.
 * \details This request gets the page number for the specified
 * address.  The ctl argument is the address and the return
 * value is the page.
 *
 * Example:
 * \code
 * mem_pageinfo_t pageinfo;
 * pageinfo.type = MEM_PAGEINO_TYPE_FLASH;
 * paginfo.num = 0;
 * //This will get the size and address of flash page 0
 * ioctl(mem_fd, I_MEM_GET_PAGEINFO, &pageinfo );
 * \endcode
 *
 * Or to lookup based on the address:
 *
 * \code
 * mem_pageinfo_t pageinfo;
 * pageinfo.type = MEM_PAGEINFO_TYPE_QUERY;
 * paginfo.addr = 0x00001000;
 * //This will get the size and address of flash page 0
 * ioctl(mem_fd, I_MEM_GETPAGEINFO, &pageinfo );
 * \endcode
 *
 * The \a type, \a num, and \a size parameters will then be populated.
 *
 * \hideinitializer
 */
#define I_MEM_GETPAGEINFO _IOCTLRW(MEM_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1, mem_pageinfo_t)
#define I_MEM_GET_PAGEINFO I_MEM_GETPAGEINFO

/*! \brief See details below
 * \details This request writes a page to flash memory.
 *
 */
#define I_MEM_WRITEPAGE _IOCTLW(MEM_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2, mem_writepage_t)

#define I_MEM_TOTAL 3

#ifdef __cplusplus
}
#endif


#endif // IFACE_DEV_MEM_H_

/*! @} */


