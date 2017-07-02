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

/*! \addtogroup DAC Digital to Analog Converter (DAC)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The DAC peripheral driver allows access to the MCU's built-in DAC. More information about accessing
 * peripheral IO is in the \ref IFACE_DEV section.
 *
 * The following is an example of how to write the DAC in an OS environment:
 *
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include "mcu/mcu.h"
 *
 * int write_dac(dac_sample_t * src, int samples){
 * 	int fd;
 * 	dac_attr_t ctl;
 *
 * 	fd = open("/dev/dac0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *		ctl.enabled_channels = (1<<0); //enable channel zero
 * 		ctl.freq = 20000; //20KHz output
 * 		ctl.pin_assign = 0; //Use GPIO configuration zero (see device specific documentation for details)
 * 		if( ioctl(fd, I_SETATTR, &ctl) < 0 ){
 * 			printf("Failed to set peripheral configuration (%d)\n", errno);
 * 			return -1;
 *		}
 *
 *		//now write the samples of the DAC
 *		lseek(fd, 0, SEEK_SET); //this sets the channel to 0, 'c' device does not auto-increment on write
 *		if ( write(fd, src, sizeof(dac_sample_t)*samples) < 0 ){
 *			printf("Error writing peripheral (%d)\n", errno);
 *			return -1;
 *		}
 * 	}
 * 	close(fd);
 * 	return 0;
 * }
 *
 * \endcode
 *
 */

/*! \file
 * \brief DAC Header File
 */

#ifndef IFACE_DEV_DAC_H_
#define IFACE_DEV_DAC_H_

#include <stdint.h>
#include "ioctl.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DAC_IOC_IDENT_CHAR 'd'


#ifndef DAC_MAX_VALUE
/*! \brief This defines the maximum value that can be written
 * to the DAC.  It is MCU specific.
 * \hideinitializer
 */
#define DAC_MAX_VALUE 0xFFC0
#endif

#ifndef DAC_SAMPLE_T
#define DAC_SAMPLE_T
/*! \brief This defines the type of variable
 * used when writing the DAC.
 */
typedef u32 dac_sample_t;
#endif


typedef mcu_action_t dac_action_t;

/*! \brief DAC Attribute Data Structure
 * \details This structure defines how the control structure
 * for configuring the DAC port.
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	u8 enabled_channels /*! \brief The DAC channels to enable */;
	u16 resd;
	u32 freq /*! \brief The output frequency */;
} dac_attr_t;

typedef struct MCU_PACK {
	u8 pin_assignment[4];
	u32 o_flags;
	u32 freq;
} dac_3_attr_t;

typedef struct MCU_PACK {
	dac_sample_t value;
	u32 channel;
} dac_reqattr_t;

/*! \brief See below for details.
 * \details This requests reads the DAC attributes.
 *
 * Example:
 * \code
 * #include <dev/adc.h>
 * dac_attr_t attr;
 * int fd;
 * ...
 * ioctl(fd, I_DAC_GETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_ATTR _IOCTLR(DAC_IOC_IDENT_CHAR, I_GLOBAL_ATTR, dac_attr_t)
#define I_DAC_GETATTR I_DAC_ATTR

/*! \brief See below for details.
 * \details This requests writes the DAC attributes.
 *
 * Example:
 * \code
 * #include <dev/adc.h>
 * dac_attr_t attr;
 * int fd;
 * ...
 * ioctl(fd, I_ADC_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_SETATTR _IOCTLW(DAC_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, dac_attr_t)
#define I_DAC_SETACTION _IOCTLW(DAC_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, dac_action_t)


/*! \brief See below for details.
 * \details This IOCTL arg value causes
 * the ioctl call to return the current output
 * value of the DAC.
 *
 * Example:
 * \code
 * dac_reqattr_t reqattr;
 * reqattr.channel = 0;
 * ioctl(dac_fd, I_DAC_GET, &reqattr);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_VALUE _IOCTLRW(DAC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL, dac_reqattr_t)
#define I_DAC_GET I_DAC_VALUE

/*! \brief See below for details.
 * \details This IOCTL arg value causes
 * the ioctl call to set the output of the DAC
 *
 * Example:
 * \code
 * dac_reqattr_t reqattr;
 * reqattr.channel = 0;
 * reqattr.value = DAC_MAX / 2;
 * ioctl(dac_fd, I_DAC_SET, &reqattr);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_SET _IOCTLW(DAC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL+1, dac_reqattr_t)

#define I_DAC_TOTAL 2

#ifdef __cplusplus
}
#endif

#endif // IFACE_DEV_DAC_H_

/*! @} */


