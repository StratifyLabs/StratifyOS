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

/*! \addtogroup SWITCHBOARD Switchboard
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The switchboard device driver
 * is a device that is used to connect
 * other devices and transfer data in
 * the background.
 *
 * For example, the switchboard can read data
 * from a UART and write it to a FIFO. It can
 * also implement UART loopback functionality.
 *
 * It is also well-suited for streaming audio
 * from one interface to another (say from
 * I2S input to a USB audio class endpoint).
 *
 * The switchboard is configured using ioctl calls.
 *
 * Reading the switchboard will read a block device
 * populated with the status of the connections.
 *
 * \code
 *
 * switchboard_status_t status;
 *
 * int fd = open("/dev/switchboard", O_RDWR);
 *
 * while( read(&status, sizeof(status)) > 0 ){
 *   printf("Connection is active:%d\n", status.o_flags != 0 );
 * }
 *
 * \endcode
 *
 * When making switchboard connections, one must consider
 * input of the throughput vs input of the output. If the output has a higher
 * throughput than the input, the terminals can be connected directly.
 * For example, a 115200 bps UART could be directly connected to a SPI at 1MHz.
 *
 * - UART (ASYNC) -> SPI (ASYNC)
 *
 * If the output and input have similar or identical throughput values, they should
 * be connected through a fifo rather than directly. This is the approach for
 * I2S to USB.
 *
 * - I2S (ASYNC) -> FFIFO (SYNC NON-BLOCKING)
 * - FFIFO (SYNC NON-BLOCKING) -> USB (ASYNC)
 *
 * The FFIFO should be configured to allow overflowing. This ensures that if the
 * USB is slightly slower than the I2S a packet will simply be dropped rather
 * than having the switchboard connection stop on a write error.
 *
 * The switchboard can also be used to buffer serial slave data in FIFOs. The UART
 * is the best example, where any bytes that arrive are written to a FIFO. But
 * the SPI and I2C can also be configured as slaves connected to FIFOs.
 *
 * UART (ASYNC) -> FIFO (SYNC NON-BLOCKING)
 *
 * The application can then poll the FIFO or receive a signal when data arrives.
 *
 * Adjusting priority must be done with care. Marking an input as high
 * priority will cause writes to the output to be executed in a high priority context.
 * This can cause problems with some drivers that share one interrupt between
 * multiple channels. For example, when connecting:
 *
 * I2S (High Priority) -> USB (Normal Priority)
 *
 * the USB will be written with a high priority. This may cause the USB
 * interrupt service routine to be interrupted causing undefined results. The solution
 * is to add a FIFO
 *
 * I2S (High Priority) -> FFIFO (SYNC NON-BLOCKING)
 * FFIFO (SYNC NON-BLOCKING) -> USB (Normal Priority)
 *
 * Using this scheme all USB channels are executed at the same priority level.
 *
 *
 *
 *
 */

/*! \file
 * \brief Switchboard Header File
 *
 */

#ifndef DEV_SOS_SWITCHBOARD_H
#define DEV_SOS_SWITCHBOARD_H

#include "sos/link.h"
#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWITCHBOARD_VERSION (0x030600)
#define SWITCHBOARD_IOC_IDENT_CHAR 'W'

typedef enum {
	SWITCHBOARD_FLAG_CONNECT /*! Configure a switchboard connection (used in o_flags of switchboard_attr_t) */ = (1<<0),
	SWITCHBOARD_FLAG_DISCONNECT /*! Clears the specified connection (used in o_flags of switchboard_attr_t) */ = (1<<1),
	SWITCHBOARD_FLAG_IS_PERSISTENT /*! Set to configure a persistent connection (used in o_flags of switchboard_attr_t) */ = (1<<2),
	SWITCHBOARD_FLAG_IS_CONNECTED /*! Used in o_flags of switchboard_connection_t for status */ = (1<<3),
	SWITCHBOARD_FLAG_IS_READING_ASYNC /*! Used in o_flags of switchboard_connection_t for status */ = (1<<4),
	SWITCHBOARD_FLAG_IS_WRITING_ASYNC /*! Used in o_flags of switchboard_connection_t for status */ = (1<<5),
	SWITCHBOARD_FLAG_IS_ERROR /*! The connection has stopped because of an error (nbyte has the error code) */ = (1<<6),
	SWITCHBOARD_FLAG_SET_TRANSACTION_LIMIT /*! Use with SWITCHBOARD_FLAG_CONNECT to specify a transaction limit other than the default */ = (1<<7),
	SWITCHBOARD_FLAG_IS_FILL_ZERO /*! If no data is available on a non-blocking input, a packet full of zeros is sent to the output */ = (1<<8),
	SWITCHBOARD_FLAG_IS_INPUT_NON_BLOCKING /*! The connection input terminal should operate in non-blocking mode (return immediately if no data is available) */ = (1<<9),
	SWITCHBOARD_FLAG_IS_OUTPUT_NON_BLOCKING /*! The connection output terminal should operate in non-blocking mode (return immediately if no data is available) */ = (1<<10),
	SWITCHBOARD_FLAG_IS_DESTROYED /*! The connection has been destroyed (this flag is self clearing) */ = (1<<11),
	SWITCHBOARD_FLAG_IS_FILL_LAST_8 /*! If no data is available on a non-blocking input, a packet is filled with the last byte of the previous packet */ = (1<<12),
	SWITCHBOARD_FLAG_IS_FILL_LAST_16 /*! If no data is available on a non-blocking input, a packet is filled with the last 16-bit word of the previous packet */ = (1<<13),
	SWITCHBOARD_FLAG_IS_FILL_LAST_32 /*! If no data is available on a non-blocking input, a packet is filled with the last 32-bit word of the previous packet */ = (1<<14),
	SWITCHBOARD_FLAG_IS_FILL_LAST_64 /*! If no data is available on a non-blocking input, a packet is filled with the last 64-bit word of the previous packet */ = (1<<15),
	SWITCHBOARD_FLAG_CLEAN /*! Cleanup connectections that have stopped on an error */ = (1<<16),
	SWITCHBOARD_FLAG_IS_CANCELED /*! Set if a connection operation was cancelled */ = (1<<17)
} switchboard_flag_t;


typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask of supported flags */;
	u16 connection_count /*! The total number of connections available */;
	u16 connection_buffer_size /*! The internal buffer size (max number of bytes for persistent connections) */;
	u32 transaction_limit /*! The maximum number of synchronous transactions that are allowed before the connection is aborted */;
	u32 resd[8];
} switchboard_info_t;

/*! \brief Switchboard Terminal
 * \details Contains the data for
 * a switchboard terminal.
 *
 */
typedef struct MCU_PACK {
	char name[LINK_NAME_MAX] /*! The name of the terminal */;
	u32 loc /*! The location of the terminal (block location or channel depending on the device) */;
	u32 bytes_transferred /*! Number of bytes transferred on the terminal */;
	s8 priority /*! Hardware interrupt priority elevation */;
	u8 device_type /*! Block or character device */;
	s16 resd; //alignment
} switchboard_terminal_t;


/*! \brief Switchboard Connection
 * \details A switchboard connection
 * defines input and output terminals
 * as well as the number of bytes to
 * transfer.
 *
 * \sa switchboard_status_t
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask flags for connection state */;
	u16 id /*! Connection id of total */;
	u16 transaction_limit /*! The maximum number of synchronous transactions that can occur before aborting */;
	switchboard_terminal_t input /*! Input device (device that is read) */;
	switchboard_terminal_t output /*! Output device (device that is written) */;
	s32 nbyte /*! Number of bytes to transfer (packet size for persisent connections); will be negative when reading to indicate an error */;
} switchboard_connection_t;

/*!
 * \brief Switchboard Status
 * \details Data type that describes the data
 * that is read from the switchboard.
 *
 * \code
 * int fd = open("/dev/switchboard", O_RDWR);
 * switchboard_status_t status;
 * read(fd, &status, sizeof(status));
 *
 * read(fd, &status, 2); //returns an error with errno set to EINVAL
 * \endcode
 *
 */
typedef switchboard_connection_t switchboard_status_t;

/*! \brief Switchboard Attributes
 * \details Switchboard attributes are used with I_SWITCHBOARD_SETATTR
 * to connect and disconnect terminals.
 *
 */
typedef switchboard_connection_t switchboard_attr_t;



#define I_SWITCHBOARD_GETVERSION _IOCTL(SWITCHBOARD_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief This request gets the SPI attributes.
 * \hideinitializer
 */
#define I_SWITCHBOARD_GETINFO _IOCTLR(SWITCHBOARD_IOC_IDENT_CHAR, I_MCU_GETINFO, switchboard_info_t)

/*! \brief See details below.
 * \hideinitializer
 *
 * \details Sets the switchboard attributes. This IOCTL
 * call is used for configuring connections on the switch board.
 *
 * \code
 * #include <sos/dev/switchboard.h>
 * #include <fcntl.h>
 * #include <unistd.h>
 *
 * int uart_fd = open("/dev/uart0", O_RDWR);
 * ioctl(uart_fd, I_UART_SETATTR); //set default attributes
 *
 * //now configure the UART in loopback mode
 * int fd = open("/dev/switchboard", O_RDWR);
 * switchboard_attr_t attr;
 * strcpy(attr.input_terminal, "uart0");
 * strcpy(attr.output_terminal, "uart0");
 * attr.o_flags = SWITCHBOARD_FLAG_CONNECT |
 *  SWITCHBOARD_FLAG_IS_PERSISTENT;
 * ioctl(fd, I_SWITCHBOARD_SETATTR, &attr); //uart0 is in loopback mode
 * close(fd); //connection persists until disconnected
 *
 * \endcode
 *
 */
#define I_SWITCHBOARD_SETATTR _IOCTLW(SWITCHBOARD_IOC_IDENT_CHAR, I_MCU_SETATTR, switchboard_attr_t)
#define I_SWITCHBOARD_SETACTION _IOCTLW(SWITCHBOARD_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#define I_SWITCHBOARD_TOTAL 1

#ifdef __cplusplus
}
#endif

#endif // DEV_SOS_SWITCHBOARD_H

/*! @} */
