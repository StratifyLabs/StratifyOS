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

/*! \addtogroup UART Universal Asynchronous Receiver/Transmitter (UART)
 * @{
 *
 * \details The UART submodule uses interrupts to send and receive data on the UART port through a circular buffer.  Once
 * the receive buffer is full, incoming bytes are dropped.  More information about accessing peripheral IO
 * is in the \ref IFACE_DEV section.
 *
 * The following is an example of how to read/write the UART in an OS environment:
 *
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include <dev/uart.h>
 *
 *
 * int read_write_uart(){
 * 	int fd;
 * 	uart_attr_t attr;
 * 	char buffer[16];
 *
 * 	fd = open("/dev/uart0", O_RDWR); //Here O_RDWR could be OR'd with O_NONBLOCK for non blocking operation
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *		attr.baudrate = 9600; //9600 Baudrate
 * 		attr.pin_assign = 0; //Use GPIO configuration zero (see device specific documentation for details)
 * 		attr.parity = UART_PARITY_NONE; //No parity
 * 		attr.start = UART_ATTR_START_BITS_1;
 * 		attr.stop = UART_ATTR_STOP_BITS_1;
 * 		attr.width = 8; //8-bit widths
 * 		if( ioctl(fd, I_SETATTR, &attr) < 0 ){
 * 			perror("failed to set attr");
 * 			return -1;
 *		}
 *
 *		//Now read or write the UART
 *		strcpy(buffer, "UART Test\n");
 *		if ( write(fd, buffer, strlen(buffer) < 0 ){ //returns after all bytes have been written
 *			perror("failed to write UART");
 *		}
 *
 *		if ( read(fd, buffer, 16) < 0 ){ //doesn't return until at least 1 bytes arrives (use O_NONBLOCK to return with whatever is available)
 *			perror("failed to read UART");
 *		}
 *
 *
 * 	}
 * 	close(fd);
 * 	return 0;
 * }
 *
 * \endcode
 *
 *
 *
 * \ingroup IFACE_DEV
 *
 */

/*! \file
 * \brief Universal Asynchronous Receiver/Transmitter Header File
 *
 */

#ifndef SOS_DEV_UART_H_
#define SOS_DEV_UART_H_

#include <stdint.h>

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_IOC_IDENT_CHAR 'u'


typedef enum {
	UART_FLAG_IS_STOP1 /*! One stop bit */ = (1<<0),
	UART_FLAG_IS_STOP2 /*! Two stop bits */ = (1<<1),
	UART_FLAG_IS_STOP0_5 /*! 0.5 stop bits */ = (1<<2),
	UART_FLAG_IS_STOP1_5 /*! 1.5 stop bits */ = (1<<3),
	UART_FLAG_IS_PARITY_NONE /*! Indicates no parity */ = (1<<4),
	UART_FLAG_IS_PARITY_ODD /*! Indicates odd parity */ = (1<<5),
	UART_FLAG_IS_PARITY_EVEN /*! Indicates even parity */ = (1<<6),
} uart_flag_t;


/*! details This list the available UART events when setting custom
 * actions for the UART callbacks.
 */
enum {
	UART_EVENT_NONE /*! No Event (used to adjust priority only) */,
	UART_EVENT_DATA_READY /*! Data is ready to be read */ = (1<<0),
	UART_EVENT_WRITE_COMPLETE /*! A write has completed */ = (1<<1)
};


#define UART_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	u32 o_flags /*! A bitmask for the supported features */;
	u32 o_events /*! Events supported by this driver */;
} uart_info_t;

/*! \brief UART Attribute Data Structure
 * \details This structure defines how the control structure
 * for opening or reconfiguring the UART port.
 */
typedef struct MCU_PACK {
	u32 o_flags;
	u32 freq;
	mcu_pin_t pin_assignment[UART_PIN_ASSIGNMENT_COUNT];
	u8 width;
} uart_attr_t;

/*! \brief This request gets the UART attributes.
 * \hideinitializer
 *
 */
#define I_UART_GETINFO _IOCTLR(UART_IOC_IDENT_CHAR, I_MCU_GETINFO, uart_info_t)

/*! \brief This request sets the UART attributes.
 * \hideinitializer
 */
#define I_UART_SETATTR _IOCTLW(UART_IOC_IDENT_CHAR, I_MCU_SETATTR, uart_attr_t)
#define I_UART_SETACTION _IOCTLW(UART_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief This request clears the receive buffer.
 */
#define I_UART_CLEAR _IOCTL(UART_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)

/*! \brief See details below.
 * \details This gets a single byte from the UART.  The request returns
 * -1 if no bytes are available to get.  Otherwise, 0 is returned and the character
 * is written to the ctl argument.
 * Example:
 * \code
 * #include <dev/uart.h>
 *
 * char c;
 * if ( ioctl(uart_fd, I_UART_GETBYTE, &c) == 0 ){
 * 	//one byte from the UART was written to c
 * }
 *
 * \endcode
 */
#define I_UART_GETBYTE _IOCTLR(UART_IOC_IDENT_CHAR, I_MCU_TOTAL + 1, char)

/*! \brief See details below.
 * \details This flushes all incoming bytes from the buffer.
 *
 * \code
 * #include <dev/uart.h>
 * ioctl(uart_fd, I_UART_FLUSH);
 * \endcode
 *
 */
#define I_UART_FLUSH _IOCTL(UART_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)

#define I_UART_TOTAL 3

#ifdef __cplusplus
}
#endif

#endif // SOS_DEV_UART_H_

/*! @} */

