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

/*! \addtogroup MLCD Mono LCD
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing monochrome LCDs.  The driver
 * allows an application to load the video memory pointer as well as the
 * width and height of the device.
 *
 * When an application accesses the LCD, it must issue a HOLD before modifying the
 * memory and a RELEASE when modification is complete.
 *
 * \code
 * int fd;
 * mlcd_attr_t attr;
 *
 * fd = open("/dev/lcd0", O_RDWR);
 * ioctl(fd, I_MCLD_GETATTR, &attr);
 * ioctl(fd, I_MLCD_HOLD, &attr);
 *  //attr.mem can be directly modified
 * ioctl(fd, I_MLCD_RELEASE, &attr);
 *  //the display will refresh based on the value of attr.freq (usually in the tens of hertz range)
 * \endcode
 */

/*! \file  */
#ifndef IFACE_DEV_MLCD_H_
#define IFACE_DEV_MLCD_H_


#include <stdint.h>
#include "iface/dev/ioctl.h"
#include "mcu/types.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MLCD_IOC_CHAR 'M'

enum {
	ORIENT_TOP /*! \brief Memory is mapped to the LCD from top (y=0) to bottom */ = (1<<0),
	ORIENT_BOTTOM /*! \brief Memory is mapped to the LCD from bottom (y=0) to top */ = (1<<1),
	ORIENT_LEFT /*! \brief Memory is mapped to the LCD from left to right */ = (1<<2),
	ORIENT_RIGHT /*! \brief Memory is mapped to the LCD from left to right */ = (1<<3)
};

/*! \brief Monochrome LCD attributes
 * \details This contains the attributes of a monochrome LCD.
 */
typedef struct MCU_PACK {
	u16 w /*! \brief LCD width in pixels */;
	u16 h /*! \brief LCD height in pixels */;
	union {
		void * mem /*! \brief A pointer to the shared LCD memory which is mapped to the screen. */;
		u8 * mem8 /*! \brief A pointer to shared LCD memory (unsigned char) */;
		u32 * mem32 /*! \brief A pointer to shared LCD memory (unsigned 32 bit) */;
	};
	union {
		void * scratch_mem /*! \brief A pointer to the shared LCD memory which is mapped to the screen. */;
		u8 * scratch_mem8 /*! \brief A pointer to shared LCD memory (unsigned char) */;
		u32 * scratch_mem32 /*! \brief A pointer to shared LCD memory (unsigned 32 bit) */;
	};
	u32 size /*! \brief The size of \a mem in bytes (typically w*h/8) */;
	u16 cols /*! \brief Number of memory 32-bit word columns */;
	u16 rows /*! \brief Number of memory rows */;
	u32 freq /*! \brief This value is R/W and specifies how often the screen is refreshed */;
	u8 hold /*! \brief This values specifies the hold count.  A non-zero hold count prevents the screen from refreshing */;
	u8 orientation /*! \brief Orientation bitmask */;
	u16 margin_left /*! \brief Left margin */;
	u16 margin_right /*! \brief Left margin */;
	u16 margin_top /*! \brief Left margin */;
	u16 margin_bottom /*! \brief Left margin */;
} mlcd_attr_t;


/*! \details This request gets the attributes of the device.
 *
 * Example:
 * \code
 * mlcd_attr_t attr;
 * ioctl(fildes, I_MLCD_GETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_ATTR _IOCTLR(MLCD_IOC_CHAR, 0, mlcd_attr_t)
#define I_MLCD_GETATTR I_MLCD_ATTR

/*! \details This request clears the LCD.  This does not
 * affect the video memory.  This is only supported on
 * devices that include a way to clear the memory using
 * a hardware command.
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_CLEAR);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_CLEAR _IOCTL(MLCD_IOC_CHAR, 1)
#define I_MLCD_CLR I_MLCD_CLEAR

/*! \details This request tells the system not to update
 * the screen because the video memory is currently being modified.  This will
 * increment the \a hold member of mlcd_attr_t (see I_MLCD_GETATTR).
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_HOLD);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_HOLD _IOCTL(MLCD_IOC_CHAR, 2)

/*! \details This request tells the system that an application
 * has modified the video memory.  The new memory will
 * be written to the LCD on the next refresh event.
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_TOUCH);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_TOUCH _IOCTL(MLCD_IOC_CHAR, 3)
#define I_MLCD_RELEASE I_MLCD_TOUCH

/*! \details This request initializes the LCD.  This request
 * is normally performed by the system meaning an application
 * will not typically call this command.
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_INIT);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_INIT _IOCTL(MLCD_IOC_CHAR, 4)

/*! \details This turns on the LCD so that an image is shown.
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_ON);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_ON _IOCTL(MLCD_IOC_CHAR, 5)

/*! \details This turns off the LCD so that no image is shown.
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_OFF);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_OFF _IOCTL(MLCD_IOC_CHAR, 6)


/*! \details Returns non-zero is the LCD is busy
 *
 * Example:
 * \code
 * ioctl(fildes, I_MLCD_BUSY);
 * \endcode
 * \hideinitializer
 */
#define I_MLCD_BUSY _IOCTL(MLCD_IOC_CHAR, 7)



#define I_MLCD_TOTAL 7


#ifdef __cplusplus
}
#endif


#endif /* IFACE_DEV_MLCD_H_ */
