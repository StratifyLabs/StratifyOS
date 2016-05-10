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

#ifndef DEV_PWM_H_
#define DEV_PWM_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_IOC_IDENT_CHAR 'p'

#ifndef PWM_DUTY_T
#define PWM_DUTY_T
/*! \brief This defines the type for the PWM duty cycle read/write
 * operations.  This definition is MCU specific.
 */
typedef uint32_t pwm_duty_t;
#endif

/*! \brief This defines a PWM sample type.
 */
typedef pwm_duty_t pwm_sample_t;

typedef mcu_action_t pwm_action_t;

/*! \details This defines the events for a PWM action.
 */
typedef enum {
	PWM_ACTION_EVENT_NONE /*! no action */ = 0,
	PWM_ACTION_EVENT_INTERRUPT /*! enable interrupt on event and execute callback */ = (1<<0),
	PWM_ACTION_EVENT_RESET /*! reset the PWM timer on channel event */ = (1<<1),
	PWM_ACTION_EVENT_STOP /*! stop the PWM timer on channel event */ = (1<<2),
	PWM_ACTION_EVENT_SETOC /*! set the OC channel pin to logic 1 */ = (1<<3),
	PWM_ACTION_EVENT_CLROC /*! clear the OC channel pin to logic 0 */ = (1<<4),
	PWM_ACTION_EVENT_TOGGLEOC /*! toggle the OC channel pin */ = (1<<5)
} pwm_action_event_t;

/*! \details This lists the flags
 * that can be set when assigning the flags member of \ref pwm_attr_t.
 */
typedef enum {
	PWM_ATTR_FLAGS_ACTIVE_HIGH /*! PWM output is high until a match and then goes low (default) */ = (1<<0),
	PWM_ATTR_FLAGS_ACTIVE_LOW /*! PWM output is low until a match and then goes high */ = (1<<1)
} pwm_attr_flags_t;


/*! \brief PWM Attribute Data Structure
 * \details This structure defines how the control structure
 * for configuring the DAC port.
 */
typedef struct MCU_PACK {
	uint8_t pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	uint8_t enabled_channels /*! \brief Enabled channels bit-mask */;
	uint16_t flags /*! Attribute flags for configuring the PWM unit */;
	pwm_duty_t top /*! \brief The top value of the PWM counter */;
	uint32_t freq /*! \brief The PWM timer frequency (target value on write, actual value on read); The period
	is the "top" member divided by "freq" */;
} pwm_attr_t;

 /*! \brief Structure to set the duty cycle of a channel.
  * \details This structure is used to
  * do a quick write of the PWM value and is used with \ref I_PWM_SET.
  */
typedef struct MCU_PACK {
	uint8_t channel /*! \brief The channel */;
	pwm_duty_t duty /*! \brief The duty cycle */;
} pwm_reqattr_t;

/*! \brief Get the PWM attributes (\ref pwm_attr_t)
 * \hideinitializer
 */
#define I_PWM_ATTR _IOCTLR(PWM_IOC_IDENT_CHAR, I_GLOBAL_ATTR, pwm_attr_t)
#define I_PWM_GETATTR I_PWM_ATTR

/*! \brief Set the PWM attributes (\ref pwm_attr_t)
 * \hideinitializer
 */
#define I_PWM_SETATTR _IOCTLW(PWM_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, pwm_attr_t)
#define I_PWM_SETACTION _IOCTLW(PWM_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, pwm_action_t)

/*! \brief See details below.
 * \details This IOCTL request sets the channel.  The ctl
 * argument is the channel to use.  For example,
 * \code
 * pwm_reqattr_t req;
 * req.channel = 0;
 * req.duty = 100;
 * ioctl(pwm_fd, I_PWM_SET, &req); //write 100 to channel 0
 * \endcode
 * \hideinitializer
 */
#define I_PWM_SET _IOCTLW(PWM_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0, pwm_duty_t)

#define I_PWM_TOTAL 1

#ifdef __cplusplus
}
#endif

#endif /* DEV_PWM_H_ */

/*! @} */
