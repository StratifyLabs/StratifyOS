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
 *
 * \endcode
 *
 *
 */

/*! \file
 * \brief Analog to Digital Converter Header File
 *
 */

#ifndef SOS_DEV_ADC_H_
#define SOS_DEV_ADC_H_

#include <stdint.h>

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \cond */
#define ADC_IOC_IDENT_CHAR 'a'
#define ADC_VERSION (0x030000)
/*! \endcond */

/*! \details ADC flags */
typedef enum {
	ADC_LOC_IS_GROUP /*! Set loc value when reading ADC to read the group) */ = (1<<30),
	ADC_FLAG_SET_CONVERTER /*! Configure the ADC */ = (1<<0),
	ADC_FLAG_IS_LEFT_JUSTIFIED /*! Left Align data */ = (1<<1),
	ADC_FLAG_IS_RIGHT_JUSTIFIED /*! Right align data */ = (1<<2),
	ADC_FLAG_SET_MASTER /*! Used with MCUs that have more than one ADC that can operate in master/slave mode */ = (1<<3),
	ADC_FLAG_SET_SLAVE /*! Used with MCUs that have more than one ADC that can operate in master/slave mode */ = (1<<4),
	ADC_FLAG_IS_TRIGGER_TMR /*! Used to trigger the ADC read on a timer event */ = (1<<5),
	ADC_FLAG_IS_TRIGGER_EINT /*! Used to trigger the ADC read on a external interrupt */ = (1<<6),
	ADC_FLAG_SET_CHANNELS /*! Configure the channels withouth changing ADC settings */ = (1<<7),
	ADC_FLAG_IS_SCAN_MODE /*! ADC will read every enabled channel when reading rather than the channel based on the location value */ = (1<<8),
	ADC_FLAG_IS_TRIGGER_EINT_EDGE_RISING = (1<<9),
	ADC_FLAG_IS_TRIGGER_EINT_EDGE_FALLING = (1<<10),
	ADC_FLAG_IS_GROUP = (1<<11),
	ADC_FLAG_IS_CONTINOUS_CONVERSION = (1<<12)
} adc_flag_t;

/*! \details ADC info for use with I_ADC_GETINFO. */
typedef struct MCU_PACK {
	u32 o_flags /*! A bitmask for the supported features */;
	u32 o_events /*! Events supported by this driver */;
	u32 freq /*! The maximum frequency */;
	u32 maximum /*! The maximum value returned by the ADC */;
	u32 reference_mv /*! The reference voltage in millivolts */;
	u8 resolution /*! The number of bits supported by the ADC */;
	u8 bytes_per_sample /*! The number of bytes in each sample */;
	u8 resd_align[2];
	u32 resd[8];
} adc_info_t;


/*! \brief ADC Pin assignment
 */
typedef struct MCU_PACK {
	mcu_pin_t channel[4] /*! Pins to use with the ADC. If more than 4 are needed, make multiple calls to I_ADC_SETATTR */;
} adc_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags /*! The flag bitmask used with the ADC */;
	adc_pin_assignment_t pin_assignment /*! The pins to assigned to the ADC */;
	u32 freq /*! Target frequency when setting ADC */;
	mcu_pin_t trigger /*! Pin or Timer trigger */;
	u8 width /*! Bit resolution (if variable resolution is supported) */;
	u8 channel_count /*! Number of channels to convert when ADC_FLAG_IS_SCAN_MODE is set */;
	u16 channel /*! Channel number when using ADC_FLAG_SET_CHANNELS with ADC_FLAG_IS_GROUP */;
	u32 rank /*! Rank when using ADC_FLAG_SET_CHANNELS with ADC_FLAG_IS_GROUP */;
	u32 sampling_time /*! Sampling time (in ADC clock cycles) when using ADC_FLAG_SET_CHANNELS with ADC_FLAG_IS_GROUP */;
	u32 resd[4];
} adc_attr_t;

#define I_ADC_GETVERSION _IOCTL(ADC_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief See below for details.
 * \details This requests reads the ADC attributes.
 *
 * Example:
 * \code
 * #include <dev/adc.h>
 * adc_attr_t attr;
 * int adc_fd;
 * ...
 * ioctl(adc_fd, I_ADC_GETINFO, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ADC_GETINFO _IOCTLR(ADC_IOC_IDENT_CHAR, I_MCU_GETINFO, adc_info_t)

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
#define I_ADC_SETATTR _IOCTLW(ADC_IOC_IDENT_CHAR, I_MCU_SETATTR, adc_attr_t)
#define I_ADC_SETACTION _IOCTLW(ADC_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

#ifdef __cplusplus
}
#endif


#endif // SOS_DEV_ADC_H_

/*! @} */

