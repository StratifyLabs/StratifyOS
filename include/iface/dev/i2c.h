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

/*! \addtogroup I2C Inter-Integrated Circuit (I2C) Master
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module implements an I2C multi-master using two GPIO pins and a pin change interrupt.  More information
 * about accessing peripheral IO is in the \ref IFACE_DEV section.
 *
 * The following is an example of how to write the I2C in an OS environment:
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include "mcu/mcu.h"
 *
 * int access_i2c(void){
 * 	int fd;
 * 	i2c_attr_t attr;
 * 	i2c_reqattr_t req;
 * 	u8 data[8];
 *
 * 	fd = open("/dev/i2c0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *		attr.bitrate = 4000000; //Use a 400KHz I2C clock frequency
 * 		attr.pin_assign = 0; //Use GPIO configuration zero (see device specific documentation for details)
 * 		if( ioctl(fd, I_SETATTR, &attr) < 0 ){
 * 			printf("Failed to set peripheral configuration (%d)\n", errno);
 * 			return -1;
 *		}
 *
 *		//Now use ioctl to setup a transfer operation
 *		req.slave_addr = 0x3F; //This is the right-justified 7-bit or 10-bit slave address
 *		req.transfer = I2C_TRANSFER_NORMAL; //This is a "normal", see i2c_trasfer_mode_t for details
 *		ioctl(fd, I_I2C_SETUP, &req);
 *
 *		//now read the I2C:  This will 1) Start 2) write the 8-bit ptr value 3) Restart 4) Read 8 bytes and 5) Stop
 *		lseek(fd, 0, SEEK_SET); //this is the ptr value
 *		if ( read(fd, data, 8) < 0 ){
 *			printf("Error Reading adc0 (%d)\n"< errno);
 *			return -1;
 *		}
 * 	}
 * 	close(fd);
 * 	return 0;
 * }
 *
 * \endcode
 */

/*! \file
 * \brief I2C Header File
 *
 */

#ifndef DEV_I2C_H_
#define DEV_I2C_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_IOC_IDENT_CHAR 'i'

#define I2C_TEN_BIT_ADDR_MASK 0xFC
#define I2C_TEN_BIT_ADDR 0xF8

/*! \details This enumeration lists the errors which may
 * occur when running an I2C operation.
 */
enum {
	I2C_STATE_NONE /*! No errors */,
	I2C_ERROR_START /*! Error while starting */,
	I2C_ERROR_WRITE /*! Error while writing */,
	I2C_ERROR_ACK /*! Ack Error (most common error for a mis-wired hardware) */,
	I2C_ERROR_STOP /*! Error while stopping */,
	I2C_ERROR_MASTER_ACK /*! The master could not create an ACK */,
	I2C_ERROR_BUS_BUSY /*! The Bus is busy (happens with multi-masters on bus) */,
	I2C_ERROR_LONG_SLEW,
	I2C_ERROR_ARBITRATION_LOST /*! Arbitration lost on multi-master bus */,
	I2C_STATE_RD_OP /*! Internal Use only */,
	I2C_STATE_WR_OP /*! Internal Use only */,
	I2C_STATE_RD_16_OP /*! Internal Use only */,
	I2C_STATE_WR_16_OP /*! Internal Use only */,
	I2C_STATE_WR_PTR_ONLY /*! Internal use only */,
	I2C_ERROR_TOTAL
};


/*! \details This lists the I2C setup transfer types.
 *
 */
typedef enum {
	I2C_TRANSFER_NORMAL /*! This specifies the IOCTL mode value for Normal.
	 * This means read() performs:
	 * - Start
	 * - Write value to ptr
	 * - Restart
	 * - Read I2C data
	 * - Stop
	 *
	 * And write() performs
	 * - Start
	 * - Write value to ptr
	 * - Write data
	 * - Stop
	 */,
	 I2C_TRANSFER_NORMAL_16  /*! \brief This specifies the IOCTL mode value for Normal but
	  * uses a 16-bit pointer rather than an 8-bit pointer value.
	  */,
	 I2C_TRANSFER_READ_ONLY  /*! \brief This specifies the IOCTL mode value for read only.
	   * This means the read() performs:
	   * - Start
	   * - Read I2C data
	   * - Stop
	   *
	   * The write() function is not available in this mode.
	   *
	   */,
	   I2C_TRANSFER_WRITE_PTR /*! \brief This will write the ptr value only without writing or reading any data. */
} i2c_transfer_t;


/*! \brief Data used to setup I2C transfers.
 * \details This is the I2C transfer setup structure.
 *
 */
typedef struct MCU_PACK {
	u16 transfer /*! \brief The I2C transfer type (\sa i2c_transfer_t) */;
	u16 slave_addr /*! \brief The slave address */;
} i2c_setup_t;

typedef i2c_setup_t i2c_reqattr_t; //legacy support

/*! \brief I2C IO Attributes
 *  \details This structure defines how the control structure
 * for configuring the I2C port.
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	u8 resd[3];
	u32 bitrate /*! \brief The I2C bitrate */;
} i2c_attr_t;


/*! \details This defines an I2C action.
 */
typedef mcu_action_t i2c_action_t;

/*! \brief This request gets the I2C attributes.
 * \hideinitializer
 */
#define I_I2C_ATTR _IOCTLR(I2C_IOC_IDENT_CHAR, I_GLOBAL_ATTR, i2c_attr_t)
#define I_I2C_GETATTR I_I2C_ATTR

/*! \brief This request sets the I2C attributes.
 * \hideinitializer
 */
#define I_I2C_SETATTR _IOCTLW(I2C_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, i2c_attr_t)

/*! \brief This request sets the I2C action.
 * \hideinitializer
 */
#define I_I2C_SETACTION _IOCTLW(I2C_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, i2c_action_t)

/*! \brief See details below.
 * \details This ioctl request sets up an I2C transfer.
 * The third argument is a pointer to an \ref i2c_reqattr_t data structure.
 *
 * \code
 * i2c_reqattr_t req;
 * char buf[32];
 * req.transfer = I2C_TRANSFER_NORMAL;
 * req.slave_addr = 0x4C;
 * ioctl(i2c_fd, I_I2C_SETUP, &req);
 * lseek(i2c_fd, loc, SEEK_SET); //loc is the location in the register map to read
 * read(i2c_fd, buf, 32);
 * \endcode
 * \hideinitializer
 */
#define I_I2C_SETUP _IOCTLW(I2C_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0, i2c_reqattr_t)

/*! \brief See details below.
 * \details This request can be used in non-blocking mode
 * to check the error status of the last transmission.  The
 * ioctl() returns the error value.  The ctl
 * argument is NULL for this request.
 * \code
 * err = ioctl(i2c_fd, I_I2C_GETERR);
 * \endcode
 *
 * \hideinitializer
 */
#define I_I2C_GETERR _IOCTL(I2C_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)

#define I_I2C_TOTAL 2


#ifdef __cplusplus
}
#endif

#endif // DEV_I2C_H_

/*! @} */

