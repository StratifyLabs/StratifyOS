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

/*! \addtogroup I2C Inter-Integrated Circuit (I2C) Master or Slave
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module implements an I2C multi-master or slave bus.
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
 * int access_i2c(){
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

#ifndef IFACE_DEV_I2C_H_
#define IFACE_DEV_I2C_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/types.h"

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
	I2C_STATE_START /*! Internal Use only */,
	I2C_STATE_RD_OP /*! Internal Use only */,
	I2C_STATE_WR_OP /*! Internal Use only */,
	I2C_STATE_RD_16_OP /*! Internal Use only */,
	I2C_STATE_WR_16_OP /*! Internal Use only */,
	I2C_STATE_WR_PTR_ONLY /*! Internal use only */,
	I2C_STATE_WR_ONLY /*! Internal use only */,
	I2C_STATE_MASTER_COMPLETE /*! Internal use only */,
	I2C_STATE_SLAVE_READ /*! Internal use only */,
	I2C_STATE_SLAVE_READ_PTR /*! Internal use only */,
	I2C_STATE_SLAVE_READ_PTR_16 /*! Internal use only */,
	I2C_STATE_SLAVE_READ_PTR_COMPLETE /*! Internal use only */,
	I2C_STATE_SLAVE_READ_COMPLETE /*! Internal use only */,
	I2C_STATE_SLAVE_WRITE /*! Internal use only */,
	I2C_STATE_SLAVE_WRITE_COMPLETE /*! Internal use only */,
	I2C_STATE_GENERAL_READ /*! Internal use only */,
	I2C_STATE_GENERAL_WRITE /*! Internal use only */,
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
	   I2C_TRANSFER_WRITE_PTR /*! \brief This will write the ptr value only without writing or reading any data. */,
	   I2C_TRANSFER_WRITE_ONLY /*! \brief This will write data without first writing the pointer information */,
	   I2C_TRANSFER_DATA_ONLY /*! \brief This will write data without first writing the pointer information */,
} i2c_transfer_t;

enum {
	I2C_EVENT_NONE /*! No Event (used to adjust priority only) */,
	I2C_EVENT_DATA_READY /*! Data is ready to be read (has been written to slave) */,
	I2C_EVENT_WRITE_COMPLETE /*! A write has completed (master has read data) */,
	I2C_EVENT_UPDATE_POINTER_COMPLETE /*! Master has written the slave data pointer */,
	I2C_EVENT_SLAVE_BUS_ERROR /*! An error occurred while addressed in slave mode */
};


/*! \brief Data used to setup I2C transfers.
 * \details This is the I2C transfer setup structure.
 *
 */
typedef struct MCU_PACK {
	u16 transfer /*! \brief The I2C transfer type (\sa i2c_transfer_t) */;
	u16 slave_addr /*! \brief The slave address */;
} i2c_setup_t;

enum {
	I2C_SLAVE_SETUP_FLAG_ALT_ADDR0 /*! If hardware supports multiple slave addrs, use the first slot (default) */ = 0,
	I2C_SLAVE_SETUP_FLAG_ALT_ADDR1 /*! If hardware supports multiple slave addrs, use the second slot */ = (1<<0),
	I2C_SLAVE_SETUP_FLAG_ALT_ADDR2 /*! If hardware supports multiple slave addrs, use the third slot */ = (1<<1),
	I2C_SLAVE_SETUP_FLAG_ALT_ADDR3 /*! If hardware supports multiple slave addrs, use the fourth slot */ = (1<<2),
	I2C_SLAVE_SETUP_FLAG_8_BIT_PTR /*! Use a 8-bit address pointer when accessing data (default) */ = 0,
	I2C_SLAVE_SETUP_FLAG_16_BIT_PTR /*! Use a 16-bit address pointer when accessing data (set automatically is size > 255) */ = (1<<3),
	I2C_SLAVE_SETUP_FLAG_ENABLE_GENERAL_CALL /*! Accept writes from general calls */ = (1<<4),
};

/*! \brief Data used to setup I2C slave transfers.
 * \details Data structure for setting up shared
 * memory registers access via an I2C slave interface.
 *
 */
typedef struct MCU_PACK {
	u8 addr /*! Slave address */;
	u8 o_flags /*! Slave flag attributes */;
	u16 size /*! Size of the data (if more than 255 set the I2C_SLAVE_SETUP_FLAG_16_BIT_PTR flag) */;
	char * data /*! Pointer to the data */;
} i2c_slave_setup_t;

typedef i2c_setup_t i2c_reqattr_t; //legacy support

enum {
	I2C_ATTR_FLAG_MASTER /*! \brief Operate as a master I2C bus */ = (0<<0),
	I2C_ATTR_FLAG_SLAVE/*! \brief Operate as a slave (ignored if master is set) */ = (1<<0),
	I2C_ATTR_FLAG_SLAVE_ACK_GENERAL_CALL /*! If slave operation, ack general call */ = (1<<1),
	I2C_ATTR_FLAG_PULLUP /*! \brief Enable internal pullups if available (ignore otherwise) */ = (1<<2)
};

/*! \brief I2C IO Attributes
 *  \details This structure defines how the control structure
 * for configuring the I2C port.
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The GPIO configuration to use */;
	u8 slave_addr /*! \brief Not used */;
	u16 o_flags /*! \brief Attribute flags */;
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
 * The third argument is a pointer to an \ref i2c_setup_t data structure.
 *
 * \code
 * i2c_setup_t setup;
 * char buf[32];
 * setup.transfer = I2C_TRANSFER_NORMAL;
 * setup.slave_addr = 0x4C;
 * ioctl(i2c_fd, I_I2C_SETUP, &setup);
 * lseek(i2c_fd, loc, SEEK_SET); //loc is the location in the register map to read
 * read(i2c_fd, buf, 32);
 * \endcode
 * \hideinitializer
 */
#define I_I2C_SETUP _IOCTLW(I2C_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0, i2c_setup_t)

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

/*! \brief See details below.
 * \details This ioctl request sets up an I2C slave memory area.
 * The third argument is a pointer to an \ref i2c_slave_setup_t data structure.
 *
 * \code
 * i2c_slave_setup_t setup;
 * char buf[32];
 * setup.data = buf;
 * setup.size = 32;
 * setup.addr = 0x4C;
 * setup.o_flags = I2C_SLAVE_SETUP_FLAG_ALT_ADDR0 | I2C_SLAVE_SETUP_FLAG_8_BIT_PTR;
 * ioctl(i2c_fd, I_I2C_SLAVE_SETUP, &setup);
 *
 * \endcode
 * \hideinitializer
 */
#define I_I2C_SLAVE_SETUP _IOCTLW(I2C_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2, i2c_slave_setup_t)


/*! \brief See details below.
 * \details This ioctl request resets the state of the I2C controller. The controller
 * will not be listening in slave mode (use I_I2C_SLAVE_SETUP to start listening).
 *
 * \hideinitializer
 */
#define I_I2C_RESET _IOCTL(I2C_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3)

#define I_I2C_TOTAL 4


#ifdef __cplusplus
}
#endif

#endif // IFACE_DEV_I2C_H_

/*! @} */

