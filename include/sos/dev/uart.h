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
 * 		//initializes using default configuration
 * 		if( ioctl(fd, I_SETATTR, 0) < 0 ){
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

#define UART_VERSION (0x030000)
#define UART_IOC_IDENT_CHAR 'u'

/*! \details UART flags used with
 * uart_attr_t.o_flags and I_UART_SETATTR.
 *
 */
typedef enum {
	UART_FLAG_SET_LINE_CODING /*! Set the control line state (baudrate, stop bits, etc) */ = (1<<0),
	UART_FLAG_IS_STOP1 /*! One stop bit (default behavior) */ = (1<<1),
	UART_FLAG_IS_STOP2 /*! Two stop bits */ = (1<<2),
	UART_FLAG_IS_STOP0_5 /*! 0.5 stop bits */ = (1<<3),
	UART_FLAG_IS_STOP1_5 /*! 1.5 stop bits */ = (1<<4),
	UART_FLAG_IS_PARITY_NONE /*! Indicates no parity (default behavior) */ = (1<<5),
	UART_FLAG_IS_PARITY_ODD /*! Indicates odd parity */ = (1<<6),
	UART_FLAG_IS_PARITY_EVEN /*! Indicates even parity */ = (1<<7),
	UART_FLAG_SET_LINE_CODING_DEFAULT /*! Sets line coding to no parity and one stop bit */ = (UART_FLAG_SET_LINE_CODING | UART_FLAG_IS_STOP1 | UART_FLAG_IS_PARITY_NONE),
	UART_FLAG_IS_RX_FIFO /*! The UART supports an receive buffer FIFO (read only using I_UART_GETINFO)  */
} uart_flag_t;


#define UART_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	u32 o_flags /*! A bitmask for the supported features */;
	u32 o_events /*! Events supported by this driver */;
	u32 size_ready /*! Number of bytes ready if a FIFO is built-in */;
	u32 size /*! Number of bytes available to the FIFO */;
	u32 resd[7];
} uart_info_t;

typedef struct MCU_PACK {
	mcu_pin_t tx;
	mcu_pin_t rx;
	mcu_pin_t rts;
	mcu_pin_t cts;
} uart_pin_assignment_t;

/*! \brief UART Attribute Data Structure
 * \details This structure defines how the control structure
 * for opening or reconfiguring the UART port.
 */
typedef struct MCU_PACK {
	u32 o_flags;
	u32 freq;
	uart_pin_assignment_t pin_assignment;
	u8 width;
	u32 resd[8];
} uart_attr_t;

#define I_UART_GETVERSION _IOCTL(UART_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_UART_GETINFO _IOCTLR(UART_IOC_IDENT_CHAR, I_MCU_GETINFO, uart_info_t)
#define I_UART_SETATTR _IOCTLW(UART_IOC_IDENT_CHAR, I_MCU_SETATTR, uart_attr_t)
#define I_UART_SETACTION _IOCTLW(UART_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This gets a single byte from the UART.  The request returns
 * -1 if no bytes are available to get.  Otherwise, 0 is returned and the character
 * is written to the ctl argument.
 * Example:
 * \code
 * #include <dev/uart.h>
 *
 * char c;
 * if ( ioctl(uart_fd, I_UART_GET, &c) == 0 ){
 * 	//one byte from the UART was written to c
 * }
 *
 * \endcode
 */
#define I_UART_GET _IOCTLR(UART_IOC_IDENT_CHAR, I_MCU_TOTAL + 0, char)

/*! \brief See details below.
 * \details This gets a single byte from the UART.  The request returns
 * -1 if no bytes are available to get.  Otherwise, 0 is returned and the character
 * is written to the ctl argument.
 * Example:
 * \code
 * #include <dev/uart.h>
 *
 * char c = 'j';
 * if ( ioctl(uart_fd, I_UART_PUT, c) == 0 ){
 * 	//one byte from the UART was written to c
 * }
 *
 * \endcode
 */
#define I_UART_PUT _IOCTL(UART_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)

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

