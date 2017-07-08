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

/*! \addtogroup PWM Pulse Width Modulation (PWM)
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module controls the PWM peripherals on a microcontroller.  More information about
 * accessing peripheral IO is in the \ref IFACE_DEV section.
 *
 * The following is an example of how to write the PWM in an OS environment:
 *
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include "mcu/mcu.h"
 *
 * int write_pwm(pwm_duty_t * src, int samples){
 * 	int fd;
 * 	pwm_attr_t ctl;
 * 	int i;
 * 	pwm_duty_t profile[50];
 *
 * 	fd = open("/dev/pwm0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *		ctl.enabled_channels = (1<<1)|(1<<0); //enable channel zero and 1 -- auto-configures the GPIO for PWM use
 * 		ctl.freq = 200000; //200KHz output
 * 		ctl.top = 10000; //The PWM period is this values divided by the frequency (10K /200K = 50ms)
 * 		ctl.pin_assign = 0; //Use GPIO configuration zero (see device specific documentation for details)
 * 		if( ioctl(fd, I_SETATTR, &ctl) < 0 ){
 * 			printf("Failed to set peripheral configuration (%d)\n", errno);
 * 			return -1;
 *		}
 *
 *		//Create a ramp-up profile
 *		for(i=0; i < 50; i++){
 *			profile[i] = i*2;
 *		}
 *
 *		//now write a profile to the PWM
 *		lseek(fd, 0, SEEK_SET); //this sets the channel to 0 -- since this is a 'B' device the file offset is unchanged by read/write
 *		if ( write(fd, profile, sizeof(pwm_duty_t)*50) < 0 ){ //This function won't return for 2.5s (50 samples * 50ms period)
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

#define PWM_IOC_IDENT_CHAR 'p'


typedef enum {
	PWM_EVENT_NONE /*! no action */ = 0,
	PWM_EVENT_INTERRUPT /*! enable interrupt on event and execute callback */ = (1<<0),
	PWM_EVENT_RESET /*! reset the PWM timer on channel event */ = (1<<1),
	PWM_EVENT_STOP /*! stop the PWM timer on channel event */ = (1<<2),
	PWM_EVENT_SETOC /*! set the OC channel pin to logic 1 */ = (1<<3),
	PWM_EVENT_CLROC /*! clear the OC channel pin to logic 0 */ = (1<<4),
	PWM_EVENT_TOGGLEOC /*! toggle the OC channel pin */ = (1<<5)
} pwm_event_t;

typedef enum {
	PWM_FLAG_IS_ACTIVE_HIGH = (1<<0),
	PWM_FLAG_IS_ACTIVE_LOW = (1<<1),
} pwm_flag_t;


typedef struct MCU_PACK {
	//provides info about the PWM
	u32 o_flags /*! Bitmask to show which flags are supported through the driver */;
	u32 o_events /*! Bitmask of supported events */;
} pwm_info_t;

#define PWM_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	u32 o_flags;
	u32 freq /*! \brief The PWM timer frequency (target value on write, actual value on read); The period is the "top" member divided by "freq" */;
	mcu_pin_t pin_assignment[PWM_PIN_ASSIGNMENT_COUNT];
	u32 top /*! \brief The top value of the PWM counter */;
} pwm_attr_t;

/*! \brief Get the PWM attributes (\ref pwm_attr_t)
 * \hideinitializer
 */
#define I_PWM_GETINFO _IOCTLR(PWM_IOC_IDENT_CHAR, I_MCU_GETINFO, pwm_info_t)

/*! \brief Set the PWM attributes (\ref pwm_attr_t)
 * \hideinitializer
 */
#define I_PWM_SETATTR _IOCTLW(PWM_IOC_IDENT_CHAR, I_MCU_SETATTR, pwm_attr_t)
#define I_PWM_SETACTION _IOCTLW(PWM_IOC_IDENT_CHAR, I_MCU_SETACTION, pwm_action_t)

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
#define I_PWM_SET _IOCTLW(PWM_IOC_IDENT_CHAR, I_MCU_TOTAL + 0, mcu_channel_t)

#define I_PWM_TOTAL 1

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_PWM_H_ */

/*! @} */
