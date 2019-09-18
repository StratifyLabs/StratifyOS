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

/*! \addtogroup PIO Pin Input/Output (PIO)
 * @{
 *
 * \ingroup IFACE_DEV
 * \details The PIO module gives access to the pins including reading, writing, and configuring
 * the direction register.  The PIO module does not allow the pin to be configured for another
 * function (such as UART TX).
 *
 *
 * The following is an example of how to use the PIO in an OS environment:
 *
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include <dev/pio.h>
 *
 *
 * int pio_example(){
 * 	int fd;
 * 	pio_attr_t attr;
 * 	pio_action_t action;
 *
 * 	fd = open("/dev/pio0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *
 *		attr.output = (1<<1)|(1<<0);
 *		attr.input = 0;
 *		//this will configure pins 0 and 1 of PIO Port 0 as outputs
 *		if ( ioctl(fd, I_PIO_SETATTR, &attr) < 0 ){
 *			printf("Failed to configure peripheral pins (%d)\n", errno);
 *		}
 *
 *		//this will set pin 0 of PIO Port 0
 *		if ( ioctl(fd, I_PIO_SETMASK, (void*)((1<<0))) ){
 *			printf("Failed to set pin (%d)\n", errno);
 *		}
 *
 *		//this will clear pin 1 of PIO 0
 *		if ( ioctl(fd, I_PIO_CLRMASK, (void*)((1<<1))) ){
 *			printf("Failed to clear pin (%d)\n", errno);
 *		}
 *
 *
 * 	}
 * 	close(fd);
 * 	return 0;
 * }
 *
 * \endcode
 * \ingroup IFACE_DEV
 *
 */

/*! \file
 * \brief General Purpose Input/Output Header File
 *
 */

#ifndef SOS_DEV_PIO_H_
#define SOS_DEV_PIO_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIO_VERSION (0x030000)
#define PIO_IOC_IDENT_CHAR 'p'

/*! \details PIO flags used with
 * pio_attr_t.o_flags and I_PIO_SETATTR.
 *
 */
typedef enum {
	PIO_FLAG_SET_INPUT /*! Configure the pin as an input */ = (1<<0),
	PIO_FLAG_SET_OUTPUT /*! Configure the pin as an output */ = (1<<1),
	PIO_FLAG_IS_PULLUP /*! Use the built-in pullup */ = (1<<2),
	PIO_FLAG_IS_PULLDOWN /*! Use the built-in pull-down */ = (1<<3),
	PIO_FLAG_IS_REPEATER /*! Use the built-in repeater function */ = (1<<4),
	PIO_FLAG_IS_FLOAT /*! Leave the pin floating */ = (1<<5),
	PIO_FLAG_IS_SPEED_LOW /*! Max speed 2Mhz (stm32f only) */ = (1<<6),
	PIO_FLAG_IS_SPEED_MEDIUM /*! Add fast mode slew rate */ = (1<<6),
	PIO_FLAG_IS_SPEED_HIGH /*! Max speed 10Mhz (stm32f only) */ = (1<<7),
	PIO_FLAG_IS_SPEED_BLAZING /*! Max speed 50Mhz (stm32f only) */ = (1<<8),
	PIO_FLAG_IS_OPENDRAIN /*! Configure as open drain */ = (1<<9),
	PIO_FLAG_IS_HYSTERESIS /*! Enable hysteresis on pin */ = (1<<10),
	PIO_FLAG_IS_DIRONLY /*! Only set input/output (ignore other settings) */ = (1<<11),
	PIO_FLAG_IS_ANALOG /*! Use an analog rather than digital input */ = (1<<12),
	PIO_FLAG_IS_INVERT /*! Invert the logic on the pin */ = (1<<13),
	PIO_FLAG_IS_FILTER /*! Filter noise on pin */ = (1<<14),
	PIO_FLAG_SET /*! Set the bits in the mask */ = (1<<15),
	PIO_FLAG_CLEAR /*! Clear the bits in the mask */ = (1<<16),
	PIO_FLAG_ASSIGN /*! Assign the pinmask value to the port */ = (1<<17)
} pio_flag_t;

typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
	u32 resd[8];
} pio_info_t;


typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_pinmask;
	u32 resd[8];
} pio_attr_t;

#define I_PIO_GETVERSION _IOCTL(PIO_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief IOCTL request to get the attributes.
 * \hideinitializer
 */
#define I_PIO_GETINFO _IOCTLR(PIO_IOC_IDENT_CHAR, I_MCU_GETINFO, pio_info_t)

/*! \brief IOCTL request to set the attributes.
 * \hideinitializer
 */
#define I_PIO_SETATTR _IOCTLW(PIO_IOC_IDENT_CHAR, I_MCU_SETATTR, pio_attr_t)

/*! \brief IOCTL request to set the action.
 * \hideinitializer
 */
#define I_PIO_SETACTION _IOCTLW(PIO_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See below for details.
 * \details This request sets the pins that are
 * sets in the mask. The ctl argument is the mask.
 * The following code sets bits 0 to 4 on port 4:
 * \code
 * pio_fd = open("/dev/pio4", O_RDWR");
 * ioctl(pio_fd, I_PIO_SETMASK, 0x0F);
 * \endcode
 * \hideinitializer
 */
#define I_PIO_SETMASK _IOCTL(PIO_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)
#define I_PIO_SET_MASK I_PIO_SETMASK

/*! \brief See below for details.
 *  \details This request clears the pins that are
 * set in the mask.  The ctl argument is the mask.  This
 * request works like \ref I_PIO_SETMASK but instead
 * of setting pins, they are cleared.
 *
 * Example:
 * \code
 * ioctl(pio_fd, I_PIO_CLRMASK, 0x0F);
 * \endcode
 * \hideinitializer
 */
#define I_PIO_CLRMASK _IOCTL(PIO_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)
#define I_PIO_CLR_MASK I_PIO_CLRMASK

/*! \brief See below for details.
 *  \details This request sets the action to take when an event
 * occurs on a PIO port.
 * The ctl argument should be of the form void (*callback)(int) where
 * the "int" argument will have a bit set for each pin that has a
 * pending event.
 *
 * Example:
 *  * \code
 * uint32_t value;
 * ioctl(pio_fd, I_PIO_GET, &value);
 * \endcode
 * \hideinitializer
 *
 */
#define I_PIO_GET _IOCTL(PIO_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)

/*! \brief See below for details.
 *  \details This request sets the action to take when an event
 * occurs on a PIO port.
 * The ctl argument should be of the form void (*callback)(int) where
 * the "int" argument will have a bit set for each pin that has a
 * pending event.
 *
 * Example:
 *  * \code
 * uint32_t value;
 * ioctl(pio_fd, I_PIO_SET, value);
 * \endcode
 * \hideinitializer
 *
 */
#define I_PIO_SET _IOCTL(PIO_IOC_IDENT_CHAR, I_MCU_TOTAL + 3)


#define I_PIO_TOTAL 4

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_PIO_H_ */

/*! @} */
