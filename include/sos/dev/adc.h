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

#define ADC_IOC_IDENT_CHAR 'a'


typedef enum {
	ADC_FLAG_IS_LEFT_JUSTIFIED = (1<<0),
	ADC_FLAG_IS_RIGHT_JUSTIFIED = (1<<1),
} adc_flag_t;

typedef struct MCU_PACK {
	u32 value;
} adc_event_t;

typedef struct MCU_PACK {
	u32 o_flags /*! A bitmask for the supported features */;
	u32 o_events /*! Events supported by this driver */;
	u8 resolution /*! The number of bits supported by the ADC */;
	u32 freq /*! The maximum frequency */;
	u32 maximum /*! The maximum value returned by the ADC */;
} adc_info_t;


typedef struct MCU_PACK {
	mcu_pin_t channel[4];
} adc_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags;
	adc_pin_assignment_t pin_assignment;
	u32 freq;
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
#define I_ADC_SETACTION _IOCTLW(ADC_IOC_IDENT_CHAR, I_MCU_SETACTION, adc_action_t)

#ifdef __cplusplus
}
#endif


#endif // SOS_DEV_ADC_H_

/*! @} */

