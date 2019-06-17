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

#ifndef SOS_DEV_DAC_H_
#define SOS_DEV_DAC_H_

#include <stdint.h>
#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAC_VERSION (0x030000)
#define DAC_IOC_IDENT_CHAR 'd'

enum dac_flags {
	DAC_FLAG_NONE = 0,
	DAC_FLAG_SET_CONVERTER = (1<<0),
	DAC_FLAG_IS_LEFT_JUSTIFIED = (1<<1),
	DAC_FLAG_IS_RIGHT_JUSTIFIED = (1<<2),
	DAC_FLAG_SET_CHANNELS = (1<<3),
	DAC_FLAG_IS_OUTPUT_BUFFERED = (1<<4),
	DAC_FLAG_IS_ON_CHIP = (1<<5),
	DAC_FLAG_IS_SAMPLE_AND_HOLD = (1<<6),
	DAC_FLAG_IS_TRIGGER_EINT = (1<<7),
	DAC_FLAG_IS_TRIGGER_EINT_RISING = (1<<8),
	DAC_FLAG_IS_TRIGGER_EINT_FALLING = (1<<9),
	DAC_FLAG_IS_TRIGGER_TMR = (1<<10)
};


typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
	u32 freq;
	u32 maximum;
	u32 reference_mv;
	u8 resolution;
	u8 bytes_per_sample;
	u16 resd0;
	u32 resd[6];
} dac_info_t;

typedef struct MCU_PACK {
	mcu_pin_t channel[4];
} dac_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags;
	dac_pin_assignment_t pin_assignment;
	u32 freq;
	mcu_pin_t trigger /*! Pin or Timer trigger */;
	u8 width;
	u8 channel;
	u32 resd[8];
} dac_attr_t;

#define I_DAC_GETVERSION _IOCTL(DAC_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief See below for details.
 * \details This requests reads the DAC attributes.
 *
 * Example:
 * \code
 * #include <dev/adc.h>
 * dac_info_t info;
 * int fd;
 * ...
 * ioctl(fd, I_DAC_GETINFO, &info);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_GETINFO _IOCTLR(DAC_IOC_IDENT_CHAR, I_MCU_GETINFO, dac_info_t)

/*! \brief See below for details.
 * \details This requests writes the DAC attributes.
 *
 * Example:
 * \code
 * #include <dev/dac.h>
 * dac_attr_t attr;
 * int fd;
 * ...
 * ioctl(fd, I_ADC_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_SETATTR _IOCTLW(DAC_IOC_IDENT_CHAR, I_MCU_SETATTR, dac_attr_t)
#define I_DAC_SETACTION _IOCTLW(DAC_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


/*! \brief See below for details.
 * \details This IOCTL arg value causes
 * the ioctl call to return the current output
 * value of the DAC.
 *
 * Example:
 * \code
 * mcu_channel_t value;
 * value.channel = 0;
 * ioctl(dac_fd, I_DAC_GET, &value);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_GET _IOCTLRW(DAC_IOC_IDENT_CHAR, I_MCU_TOTAL, mcu_channel_t)

/*! \brief See below for details.
 * \details This IOCTL arg value causes
 * the ioctl call to set the output of the DAC
 *
 * Example:
 * \code
 * mcu_channel_t value;
 * value.channel = 0;
 * value.value = DAC_MAX / 2;
 * ioctl(dac_fd, I_DAC_SET, &value);
 * \endcode
 * \hideinitializer
 */
#define I_DAC_SET _IOCTLW(DAC_IOC_IDENT_CHAR, I_MCU_TOTAL+1, mcu_channel_t)

#define I_DAC_TOTAL 2

#ifdef __cplusplus
}
#endif

#endif // SOS_DEV_DAC_H_

/*! @} */


