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

/*! \addtogroup ADC Analog to Digital Converter (ADC)
 * ADC Interface Spec
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The ADC peripheral driver allows access to the MCU's built-in ADC.  Each peripheral
 * is opened using a unique port number and mode settings.  Once open, any channel from the ADC can be read.
 * Closing the peripheral will disable power to the ADC.  More information about accessing hardware is
 * in the \ref IFACE_DEV section.
 *
 * The following is an example of how to read the ADC in an OS environment:
 *
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include <dev/adc.h>
 *
 * int read_adc(void){
 * 	int fd;
 * 	adc_sample_t buffer[16]; //a buffer for 16 samples
 * 	adc_attr_t attr;
 *
 * 	fd = open("/dev/adc0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *		attr.enabled_channels = (1<<5); //enabled channel five
 * 		attr.freq = ADC_MAX_FREQ;
 * 		attr.pin_assign = 0;
 * 		if( ioctl(fd, I_ADC_SETATTR, &ctl_adc) < 0 ){
 * 			printf("Failed to set ADC configuration (%d)\n", errno);
 * 			return -1;
 *		}
 *
 *		//now read 16 samples of the ADC
 *		lseek(fd, 5, SEEK_SET); //this sets the channel to 5 -- it's a 'c' device so it doesn't auto-increment
 *		if ( read(fd, buffer, sizeof(adc_sample_t)*16) < 0 ){
 *			printf("Error reading peripheral (%d)\n", errno);
 *			return -1;
 *		}
 * 	}
 * 	close(fd);
 * 	return 0;
 * }
 *
 * \endcode
 *
 *
 */

/*! \file
 * \brief Analog to Digital Converter Header File
 *
 */

#ifndef DEV_ADC_H_
#define DEV_ADC_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"
#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_IOC_IDENT_CHAR 'a'

typedef mcu_action_t adc_action_t;

#ifndef ADC_MAX
/*! \brief This defines the maximum value the ADC will return.  It is
 * specific to the MCU architecture.
 * \hideinitializer
 */
#define ADC_MAX 0x0000FFF0
#endif

#ifndef ADC_MIN
/*! \brief This defines the minimum step value of the ADC.  This
 * is MCU specific.  For example if the data is not right-justified
 * and the lowest 4 bits are always zero, ADC_MIN will be 0x10.
 * \hideinitializer
 */
#define ADC_MIN 0x01
#endif


#ifndef ADC_SAMPLE_T
/*! \brief This defines the type of an ADC sample (see
 * adc_read() and adc_write()).
 */
typedef u32 adc_sample_t;
#endif


#ifndef ADC_MAX_FREQ
/*! \brief This values defines the maximum frequency
 * that can be used when reading the ADC.  When
 * doing an IOCTL write request, the freq member
 * should not exceed this value.
 * \hideinitializer
 */
#define ADC_MAX_FREQ 200000
#endif

/*! \brief ADC IO Attributes
 * \details This structure defines the attributes structure
 * for configuring the ADC port.
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The pin assignment to use (see \ref LPC17XXDEV) */;
	u8 resd;
	u16 enabled_channels /*! \brief A mask of the enabled channels (a one means the channel is enabled) */;
	u32 freq /*! \brief The read update frequency (can't be zero).  This is a target value when
	writing and the actual value when reading. */;
} adc_attr_t;

/*! \brief See below for details.
 * \details This requests reads the ADC attributes.
 *
 * Example:
 * \code
 * #include <dev/adc.h>
 * adc_attr_t attr;
 * int adc_fd;
 * ...
 * ioctl(adc_fd, I_ADC_GETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ADC_ATTR _IOCTLR(ADC_IOC_IDENT_CHAR, I_GLOBAL_ATTR, adc_attr_t)
#define I_ADC_GETATTR I_ADC_ATTR

/*! \brief See below for details.
 * \details This requests writes the ADC attributes.
 *
 * Example:
 * \code
 * #include <dev/adc.h>
 * adc_attr_t attr;
 * int adc_fd;
 * ...
 * ioctl(adc_fd, I_ADC_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ADC_SETATTR _IOCTLW(ADC_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, adc_attr_t)
#define I_ADC_SETACTION _IOCTLW(ADC_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, adc_action_t)

#ifdef __cplusplus
}
#endif


#endif // DEV_ADC_H_

/*! @} */

