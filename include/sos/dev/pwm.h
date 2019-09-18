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

/*! \addtogroup PWM Pulse Width Modulation (PWM)
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module controls the PWM peripherals on a microcontroller.  More information about
 * accessing peripheral IO is in the \ref IFACE_DEV section.
 *
 *
 *
 */

/*! \file
 * \brief Pulse Width Modulation Header File
 *
 */

#ifndef SOS_DEV_PWM_H_
#define SOS_DEV_PWM_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_VERSION (0x030000)
#define PWM_IOC_IDENT_CHAR 'p'

/*! \details PWM flags used with
 * pwm_attr_t.o_flags and I_PMW_SETATTR.
 *
 */
typedef enum {
	PWM_FLAG_SET_TIMER /*! Set to configure the frequency and period (implies PWM_FLAG_SET_CHANNEL) */ = (1<<0),
	PWM_FLAG_IS_ACTIVE_HIGH /*! Set to configure the outputs as active high */ = (1<<1),
	PWM_FLAG_IS_ACTIVE_LOW /*! Set to configure the outputs as active low */ = (1<<2),
	PWM_FLAG_SET_CHANNELS /*! Set to just configure channels and leave the timer alone */ = (1<<3),
	PWM_FLAG_CLEAR_CHANNELS /*! Set to clear all channels; may be with with PWM_FLAG_SET_CHANNELS or PMW_FLAG_SET_TIMER  */ = (1<<4),
	PWM_FLAG_IS_ENABLED /*! Set with PMW_FLAG_SET_TIMER to enable the timer */ = (1<<5)
} pwm_flag_t;


typedef struct MCU_PACK {
	//provides info about the PWM
	u32 o_flags /*! Bitmask to show which flags are supported through the driver */;
	u32 o_events /*! Bitmask of supported events */;
	u32 resd[8];
} pwm_info_t;

typedef struct MCU_PACK {
	mcu_pin_t channel[4];
} pwm_pin_assignment_t;

/*! \details PWM Attributes */
typedef struct MCU_PACK {
	u32 o_flags /*! Or'd flags such as PWM_FLAG_SET_TIMER */;
	pwm_pin_assignment_t pin_assignment;
	u32 freq /*! The PWM counter frequency */;
	u32 period /*! The PWM period in counts */;
	mcu_channel_t channel /*! Channel when using PWM_FLAG_SET_CHANNELS */;
	u32 resd[8];
} pwm_attr_t;

#define I_PWM_GETVERSION _IOCTL(PWM_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief Get the PWM attributes (\ref pwm_attr_t)
 * \hideinitializer
 */
#define I_PWM_GETINFO _IOCTLR(PWM_IOC_IDENT_CHAR, I_MCU_GETINFO, pwm_info_t)

/*! \brief Set the PWM attributes (\ref pwm_attr_t)
 * \hideinitializer
 */
#define I_PWM_SETATTR _IOCTLW(PWM_IOC_IDENT_CHAR, I_MCU_SETATTR, pwm_attr_t)
#define I_PWM_SETACTION _IOCTLW(PWM_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This IOCTL request sets the channel.  The ctl
 * argument is the channel to use.  For example,
 * \code
 * mcu_channel_t value;
 * value.channel = 0;
 * value.value = 100;
 * ioctl(pwm_fd, I_PWM_SET, &value); //write 100 to channel 0
 * \endcode
 * \hideinitializer
 */
#define I_PWM_SETCHANNEL _IOCTLW(PWM_IOC_IDENT_CHAR, I_MCU_TOTAL + 0, mcu_channel_t)
#define I_PWM_GETCHANNEL _IOCTLRW(PWM_IOC_IDENT_CHAR, I_MCU_TOTAL + 1, mcu_channel_t)
#define I_PWM_SET _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)
#define I_PWM_GET _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 3)
#define I_PWM_ENABLE _IOCTL(PWM_IOC_IDENT_CHAR, I_MCU_TOTAL + 4)
#define I_PWM_DISABLE _IOCTL(PWM_IOC_IDENT_CHAR, I_MCU_TOTAL + 5)

#define I_PWM_TOTAL 6

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_PWM_H_ */

/*! @} */
