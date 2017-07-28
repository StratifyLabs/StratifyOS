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

#ifndef SOS_DEV_LED_H_
#define SOS_DEV_LED_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LED_IOC_IDENT_CHAR 'L'

typedef struct MCU_PACK {
	u32 value;
} led_event_t;


enum {
	LED_FLAG_NONE = (1<<0),
	LED_FLAG_IS_HIGH_IMPEDANCE /*! LED driver disables output when LED is off */ = (1<<1),
	LED_FLAG_IS_DUTY_CYCLE /*! When set, use \a duty_cycle and \a period to set brightness or flashing */ = (1<<2),
	LED_FLAG_ENABLE /*! Turn the LED on */ = (1<<3),
	LED_FLAG_DISABLE /*! Turn the LED off (use with LED_FLAG_IS_HIGH_IMPEDANCE) */ = (1<<4),
};

/*! \brief I2S IO Attributes
 *  \details This structure defines how the control structure
 * for configuring the I2S port.
 */
typedef struct MCU_PACK {
	u32 o_flags  /*!  Flags that are supported by the driver */;
	u32 o_events  /*! Events that are supported by the driver */;
} led_info_t;

#define LED_PIN_ASSIGNMENT_COUNT 1

typedef struct MCU_PACK {
	u32 o_flags  /*! Flag bitmask */;
	u32 duty_cycle /*! LED duty cycle (if supported) */;
	u32 period /*! LED blinking period in us (if supported) */;
} led_attr_t;

/*! \brief This request gets the LED info.
 * \hideinitializer
 */
#define I_LED_GETINFO _IOCTLR(LED_IOC_IDENT_CHAR, I_MCU_GETINFO, led_info_t)

/*! \brief This request sets the LED attributes.
 * \hideinitializer
 */
#define I_LED_SETATTR _IOCTLW(LED_IOC_IDENT_CHAR, I_MCU_SETATTR, led_attr_t)

/*! \brief This request sets the I2S action.
 * \hideinitializer
 */
#define I_LED_SETACTION _IOCTLW(LED_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#define I_LED_TOTAL 0


#ifdef __cplusplus
}
#endif



#endif /* SOS_DEV_LED_H_ */
