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

/*! \addtogroup TMR Timer (TMR)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Timer module contains functions to access an MCU's timer/counters as well as
 * input capture and output compare units.  More information about accessing peripheral IO is in
 * the \ref IFACE_DEV section.
 *
 * The following is an example of how to use the timer in an OS environment:
 *
 * \code
 *
 * \endcode
 *
 *
 */

/*! \file
 * \brief Timer Header File
 *
 */

#ifndef SOS_DEV_TMR_H_
#define SOS_DEV_TMR_H_

#include <stdint.h>

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TMR_VERSION (0x030000)
#define TMR_IOC_IDENT_CHAR 't'


enum {
	TMR_FLAG_SET_TIMER /*! Configure the timer attributes */ = (1<<0),
	TMR_FLAG_IS_SOURCE_CPU /*! Use the CPU as the source for the clock (timer mode) */ = (1<<1),
	TMR_FLAG_IS_SOURCE_IC0 /*! Use input capture channel 0 for the clock source (counter mode) */ = (1<<2),
	TMR_FLAG_IS_SOURCE_IC1 /*! Use input capture channel 1 for the clock source (counter mode) */ = (1<<3),
	TMR_FLAG_IS_SOURCE_IC2 /*! Use input capture channel 2 for the clock source (counter mode) */ = (1<<4),
	TMR_FLAG_IS_SOURCE_IC3 /*! Use input capture channel 3 for the clock source (counter mode) */ = (1<<5),
	TMR_FLAG_IS_SOURCE_EDGERISING /*! Count rising edges */ = (1<<6),
	TMR_FLAG_IS_SOURCE_EDGEFALLING /*! Count falling edges */ = (1<<7),
	TMR_FLAG_IS_SOURCE_EDGEBOTH /*! Count both edges */ = (1<<8),
	TMR_FLAG_IS_SOURCE_COUNTDOWN /*! Count down (not up) */ = (1<<9),
	TMR_FLAG_IS_AUTO_RELOAD /*! Auto reload the timer */ = (1<<10),
	TMR_FLAG_SET_CHANNEL /*! Configure channel characteristics */ = (1<<11),
	TMR_FLAG_IS_CHANNEL_EDGERISING /*! Count rising edges */ = TMR_FLAG_IS_SOURCE_EDGERISING,
	TMR_FLAG_IS_CHANNEL_EDGEFALLING /*! Count falling edges */ = TMR_FLAG_IS_SOURCE_EDGEFALLING,
	TMR_FLAG_IS_CHANNEL_EDGEBOTH /*! Count both edges */ = TMR_FLAG_IS_SOURCE_EDGEBOTH,
	TMR_FLAG_IS_CHANNEL_STOP_ON_RESET /*! Stop when the timer resets */ = (1<<12),
	TMR_FLAG_IS_CHANNEL_RESET_ON_MATCH /*! Reset when the timer finds a match */ = (1<<13),
	TMR_FLAG_IS_CHANNEL_STOP_ON_MATCH /*! Stop when the timer finds a match */ = (1<<14),
	TMR_FLAG_IS_CHANNEL_SET_OUTPUT_ON_MATCH /*! Set the output to 1 on a match */ = (1<<15),
	TMR_FLAG_IS_CHANNEL_CLEAR_OUTPUT_ON_MATCH /*! Clear the output to 0 on a match */ = (1<<16),
	TMR_FLAG_IS_CHANNEL_TOGGLE_OUTPUT_ON_MATCH /*! Toggle the output on a match */ = (1<<17),
	TMR_FLAG_IS_CHANNEL_PWM_MODE /*! Put the timer in PWM mode */ = (1<<18),
	TMR_FLAG_SET_TRIGGER /*! Set the trigger for the timer */ = (1<<19),
	TMR_FLAG_IS_SLAVE /*! The trigger indicates slave behavior */ = (1<<20),
	TMR_FLAG_IS_MASTER /*! The trigger indicates master mode */ = (1<<21),
	TMR_FLAG_IS_TRIGGER_RELOAD /*! Trigger when timer reloads */ = (1<<22),
	TMR_FLAG_IS_TRIGGER_RESET /*! Trigger when timer is re-initialized */ = (1<<23),
	TMR_FLAG_IS_TRIGGER_ENABLE /*! Trigger when timer is enabled*/ = (1<<24),
	TMR_FLAG_IS_TRIGGER_OC0 /*! Trigger on output compare 0 */ = (1<<25),
	TMR_FLAG_IS_TRIGGER_OC1 /*! Trigger on output compare 1 */ = (1<<26),
	TMR_FLAG_IS_TRIGGER_OC2 /*! Trigger on output compare 2 */ = (1<<27),
	TMR_FLAG_IS_TRIGGER_OC3 /*! Trigger on output compare 3 */ = (1<<28),
};

typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
	u32 freq;
	u32 resd[8];
} tmr_info_t;

typedef struct MCU_PACK {
	mcu_pin_t channel[4];
} tmr_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags;
	tmr_pin_assignment_t pin_assignment;
	u32 freq /*! The timer frequency */;
	u32 period /*! If this value is non-zero, the timer will reset when it matches this value (if supported) */;
	mcu_channel_t channel;
	u32 resd[8];
} tmr_attr_t;

#define I_TMR_GETVERSION _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_TMR_GETINFO _IOCTLR(TMR_IOC_IDENT_CHAR, I_MCU_GETINFO, tmr_info_t)
#define I_TMR_SETATTR _IOCTLW(TMR_IOC_IDENT_CHAR, I_MCU_SETATTR, tmr_attr_t)
#define I_TMR_SETACTION _IOCTLW(TMR_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This request writes the value of the
 * current output compare channel. The ctl
 * argument is a pointer to the mcu_channel_t structure.
 * For example:
 *  * \code
 * mcu_channel_t chan_req;
 * chan_req.channel = 1; //write to OC channel 1
 * chan_req.value = 1000000; //write 1 million to the OC channel
 * if ( ioctl(tmr_fd, I_TMR_SETCHANNEL, &chan_req ){
 * 	//There was an error
 * 	return -1;
 * }
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SETCHANNEL _IOCTLW(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 0, mcu_channel_t)

/*! \brief See details below.
 * \details This request reads the value of the
 * current output compare channel. The ctl
 * argument is a pointer to a mcu_channel_t structure.  tmr_ioctl() will return
 * zero on success.  For example:
 * \code
 * mcu_channel_t chan_req;
 * int oc_value;
 * chan_req.channel = 1;
 * if ( ioctl(tmr_fd, I_TMR_GETOC, &chan_req ){
 * 	//There was an error
 * 	return -1;
 * }
 * oc_value = chan_req.value;
 * \endcode
 * \hideinitializer
 */
#define I_TMR_GETCHANNEL _IOCTLRW(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 1, mcu_channel_t)

/*! \brief See details below.
 * \details This request writes the value of the timer.
 * For example:
 * \code
 * u32 value = 1000000;
 * ioctl(tmr_fd, I_TMR_SET, value);
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SET _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)

/*! \brief See details below.
 * \details This request reads the value of the timer.
 * For example:
 * \code
 * u32 value;
 * value = ioctl(tmr_fd, I_TMR_GET);
 * \endcode
 * \hideinitializer
 */
#define I_TMR_GET _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 3)
#define I_TMR_ENABLE _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 4)
#define I_TMR_DISABLE _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 5)

#define I_TMR_TOTAL 6

#ifdef __cplusplus
}
#endif

#endif // SOS_DEV_TMR_H_

/*! @} */

