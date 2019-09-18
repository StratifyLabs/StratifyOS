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

/*! \addtogroup I2C Inter-Integrated Circuit (I2C) Master or Slave
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module implements an I2C multi-master or slave bus.
 *
 * The following is an example of how to write the I2C in an OS environment:
 * ```
 * //md2code:include
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <sos/dev/i2c.h>
 * ```
 *
 * ```
 * //md2code:main
 *
 * int fd;
 * i2c_attr_t attr;
 * u8 data[8];
 *
 * fd = open("/dev/i2c0", O_RDWR);
 * if ( fd < 0 ){
 *   printf("Error opening peripheral (%d)\n", errno);
 * } else {
 *   attr.o_flags = I2C_FLAG_SET_MASTER;
 *   attr.freq = 4000000; //Use a 400KHz I2C clock frequency
 *   if( ioctl(fd, I_I2C_SETATTR, &attr) < 0 ){
 *     printf("Failed to set peripheral configuration (%d)\n", errno);
 *     return -1;
 *   }
 *
 *   //Now use ioctl to setup a transfer operation
 *   attr.slave_addr[0].addr8[0] = 0x3F; //This is the right-justified 7-bit or 10-bit slave address
 *   attr.o_flags = I2C_FLAG_PREPARE_PTR_DATA; //This is a "normal", see i2c_trasfer_mode_t for details
 *   ioctl(fd, I_I2C_SETATTR, &attr);
 *
 *   //now read the I2C:  This will 1) Start 2) write the 8-bit ptr value 3) Restart 4) Read 8 bytes and 5) Stop
 *   lseek(fd, 0, SEEK_SET); //this is the ptr value
 *	  if ( read(fd, data, 8) < 0 ){
 *     printf("Error Reading adc0 (%d)\n", errno);
 *     return -1;
 *   }
 *   close(fd);
 * }
 *
 *
 * ```
 */

/*! \file
 * \brief I2C Header File
 *
 */

#ifndef SOS_DEV_I2C_H_
#define SOS_DEV_I2C_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_VERSION (0x030000)
#define I2C_IOC_IDENT_CHAR 'i'

#define I2C_TEN_BIT_ADDR_MASK 0xFC
#define I2C_TEN_BIT_ADDR 0xF8

/*! \details This enumeration lists the errors which may
 * occur when running an I2C operation.
 */
enum i2c_error {
	I2C_ERROR_NONE /*! No errors */,
	I2C_ERROR_START /*! Error while starting */,
	I2C_ERROR_WRITE /*! Error while writing */,
	I2C_ERROR_ACK /*! Ack Error (most common error for a mis-wired hardware) */,
	I2C_ERROR_STOP /*! Error while stopping */,
	I2C_ERROR_MASTER_ACK /*! The master could not create an ACK */,
	I2C_ERROR_BUS_BUSY /*! The Bus is busy (happens with multi-masters on bus) */,
	I2C_ERROR_LONG_SLEW,
	I2C_ERROR_ARBITRATION_LOST /*! Arbitration lost on multi-master bus */,
	I2C_ERROR_TIMEOUT /*! Timeout error */,
	I2C_ERROR_OVERFLOW /*! Overflow error */,
};

/*! \brief I2C Info Data Structure */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask of supported flags */;
	u32 o_events /*! Bitmask of supported events */;
	u32 freq /*! Maximum supported bitrate */;
	u32 err /*! The error of the most recent transaction */;
	u32 resd[8] /*! Reserved */;
} i2c_info_t;

/*! \details I2C flags used with
 * i2c_attr_t.o_flags and I_I2C_SETATTR.
 *
 */
typedef enum {
	I2C_FLAG_NONE /*! No Flags */ = 0,
	I2C_FLAG_SET_MASTER /*! Operate as a master I2C bus */ = (1<<0),
	I2C_FLAG_SET_SLAVE/*! Operate as a slave (ignored if master is set) */ = (1<<1),
	I2C_FLAG_IS_SLAVE_ACK_GENERAL_CALL /*! If slave operation, ack general call */ = (1<<2),
	I2C_FLAG_IS_PULLUP /*! Enable internal pullups if available (ignore otherwise) */ = (1<<3),
	I2C_FLAG_PREPARE_PTR_DATA /*! This prepares the driver to write the ptr then read/write data */ = (1<<4),
	I2C_FLAG_IS_PTR_16 /*! This tells the driver to write a 16-bit ptr then read/write data (use with I2C_FLAG_PREPARE_PTR_DATA or I2C_FLAG_PREPARE_PTR)  */ = (1<<5),
	I2C_FLAG_PREPARE_PTR /*! This will write the ptr value only without writing or reading any data. */ = (1<<6),
	I2C_FLAG_PREPARE_DATA /*! This will read/write data without first writing the pointer information */ = (1<<7),
	I2C_FLAG_IS_SLAVE_ADDR0 /*! If hardware supports multiple slave addrs, use the first slot (default) */ = (1<<8),
	I2C_FLAG_IS_SLAVE_ADDR1 /*! If hardware supports multiple slave addrs, use the second slot */ = (1<<9),
	I2C_FLAG_IS_SLAVE_ADDR2 /*! If hardware supports multiple slave addrs, use the third slot */ = (1<<10),
	I2C_FLAG_IS_SLAVE_ADDR3 /*! If hardware supports multiple slave addrs, use the fourth slot */ = (1<<11),
	I2C_FLAG_IS_SLAVE_PTR_8 /*! Use a 8-bit address pointer when accessing data (default) */ = (1<<12),
	I2C_FLAG_IS_SLAVE_PTR_16 /*! Use a 16-bit address pointer when accessing data (set automatically is size > 255) */ = (1<<13),
	I2C_FLAG_RESET /*! Reset the state of the I2C */ = (1<<14),
	I2C_FLAG_STRETCH_CLOCK = (1<<15),
	I2C_FLAG_IS_NO_STOP /*! Don't issue a stop condition when complete (use with I2C_FLAG_PREPARE_DATA) */ = (1<<16)
} i2c_flag_t;

typedef union {
	u8 addr8[2];
	u16 addr16;
} i2c_slave_addr_t;

/*! \brief I2C Pin Assignment
 */
typedef struct MCU_PACK {
	mcu_pin_t sda /*! Data pin */;
	mcu_pin_t scl /*! Clock pin */;
} i2c_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags /*! Attribute flags */;
	i2c_pin_assignment_t pin_assignment /*! The pin assignment */;
	u32 freq /*! The bit frequency */;
	i2c_slave_addr_t slave_addr[2] /*! Slave address */;
	u32 size /*! Memory size when setting up slave */;
	void * data /*! Memory pointer to slave data */;
	u32 resd[8];
} i2c_attr_t;

#define I_I2C_GETVERSION _IOCTL(I2C_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief This request gets the I2C attributes.
 * \hideinitializer
 */
#define I_I2C_GETINFO _IOCTLR(I2C_IOC_IDENT_CHAR, I_MCU_GETINFO, i2c_info_t)

/*! \brief This request sets the I2C attributes.
 * \hideinitializer
 */
#define I_I2C_SETATTR _IOCTLW(I2C_IOC_IDENT_CHAR, I_MCU_SETATTR, i2c_attr_t)

/*! \brief This request sets the I2C action.
 * \hideinitializer
 */
#define I_I2C_SETACTION _IOCTLW(I2C_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

#define I_I2C_TOTAL 0


#ifdef __cplusplus
}
#endif

#endif // SOS_DEV_I2C_H_

/*! @} */

