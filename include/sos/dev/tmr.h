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
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include "mcu/mcu.h"
 *
 * int my_tmr_action(void * context, u32 value){
 * 	//take some action on an eint event -- this runs in priveleged, handler mode, it should be fast and short
 * 	return 1; //return non-zero to keep the action between successive events
 * }
 *
 * int tmr_example(){
 * 	int fd;
 * 	tmr_attr_t attr;
 * 	mcu_action_t action;
 * 	mcu_channel_t chan_req;
 *
 * 	fd = open("/dev/tmr0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		perror("Failed to open timer");
 * 	} else {
 *
 * 		//This configuration sets a timer at 1MHz using the CPU as the clock source
 *		attr.pin_assign = 0;
 *		attr.freq = 1000000; //Run the timer at 1MHz (if CPU is fast enough) -- this is not applicable in counter mode
 *		attr.clksrc = TMR_CLKSRC_CPU;
 *		attr.enabled_ic_chans = 0; //Setting bits here will configure the GPIO for input capture (depending on pin_assign)
 *		attr.enabled_oc_chans = 0; //Setting bits here will configure the GPIO for output compare (depending on pin_assign)
 * 		if( ioctl(fd, I_SETATTR, &attr) < 0 ){
 * 			perror("Failed to set attr");
 * 			return -1;
 *		}
 *
 *		//Now we will set up the counter to reset at 1 million counts using an output compare unit (no GPIO is affected)
 *		chan_req.channel = TMR_ACTION_CHANNEL_OC0;
 *		chan_req.value = 1000000;
 *		if ( ioctl(fd, I_TMR_SET_OC, &chan_req) ){ //sets the value of OC0 to 1 million
 *			perror("failed to set OC");
 *		}
 *
 *		action.channel = TMR_ACTION_CHANNEL_OC0; //This will output compare 0 -- MCU specific definition
 *		action.o_events = TMR_EVENT_RESET;
 *		action.handler.callback = NULL;
 *		action.handler.context = NULL;
 *		if ( ioctl(fd, I_SETACTION, &action) ){ //cause the timer to reset to zero when it hits 1 million
 *			perror("failed to set action");
 *		}
 *
 *		//Have the timer start counting
 *		if ( ioctl(fd, I_TMR_ON) ){
 *			 perror("failed to enable timer");
 *		}
 *
 * 	}
 * 	close(fd); //This turns off power to the timer
 * 	return 0;
 * }
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

#define TMR_IOC_IDENT_CHAR 't'


/*! \details This defines no action
 * for an OC/IC event.
 */
typedef enum {
	TMR_EVENT_NONE /*! no action */ = 0,
	TMR_EVENT_INTERRUPT /*! enable interrupt on IC/OC event */ = (1<<0),
	TMR_EVENT_RESET /*! reset the timer on IC/OC event */ = (1<<1),
	TMR_EVENT_STOP /*! stop the timer on OC/IC event */ = (1<<2),
	TMR_EVENT_SETOC /*! set the OC channel pin to logic 1 */ = (1<<3),
	TMR_EVENT_CLROC /*! clear the OC channel pin to logic 0 */ = (1<<4),
	TMR_EVENT_TOGGLEOC /*! toggle the OC channel pin */ = (1<<5),
	TMR_EVENT_PWMMODE /*! Execute OC channel in PWM Mode */ = (1<<6),
} tmr_event_t;

enum {
	TMR_FLAG_CLKSRC_CPU /*! Use the CPU as the source for the clock (timer mode) */ = (1<<0),
	TMR_FLAG_CLKSRC_IC0 /*! Use input capture channel 0 for the clock source (counter mode) */ = (1<<1),
	TMR_FLAG_CLKSRC_IC1 /*! Use input capture channel 1 for the clock source (counter mode) */ = (1<<2),
	TMR_FLAG_CLKSRC_IC2 /*! Use input capture channel 2 for the clock source (counter mode) */ = (1<<3),
	TMR_FLAG_CLKSRC_IC3 /*! Use input capture channel 3 for the clock source (counter mode) */ = (1<<4),
	TMR_FLAG_CLKSRC_EDGERISING /*! Count rising edges */ = (1<<5),
	TMR_FLAG_CLKSRC_EDGEFALLING /*! Count falling edges */ = (1<<6),
	TMR_FLAG_CLKSRC_EDGEBOTH /*! Count both edges */ = (1<<7),
	TMR_FLAG_CLKSRC_COUNTDOWN /*! Count down (not up) */ = (1<<8),
	TMR_FLAG_AUTO_RELOAD /*! Auto reload the time */ = (1<<9),
	TMR_FLAG_ENABLE_OC /*! Auto reload the time */ = (1<<10),
	TMR_FLAG_ENABLE_IC /*! Auto reload the time */ = (1<<11),
	TMR_FLAG_ENABLE /*! Turn the timer on */ = (1<<12),
	TMR_FLAG_DISABLE /*! Turn the timer off */ = (1<<13),
};

typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
	u32 freq;
} tmr_info_t;

#define TMR_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	u32 o_flags;
	mcu_pin_t pin_assignment[TMR_PIN_ASSIGNMENT_COUNT];
	u32 freq;
} tmr_attr_t;


/*! \brief This request gets the timer attributes.
 * \hideinitializer
 *
 */
#define I_TMR_GETINFO _IOCTLR(TMR_IOC_IDENT_CHAR, I_MCU_GETINFO, tmr_info_t)

/*! \brief This request sets the timer attributes.
 * \hideinitializer
 */
#define I_TMR_SETATTR _IOCTLW(TMR_IOC_IDENT_CHAR, I_MCU_SETATTR, tmr_attr_t)

/*! \brief This request sets the timer interrupt action.
 * \hideinitializer
 */
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
 * if ( ioctl(tmr_fd, I_TMR_SETOC, &mcu_channel_t ){
 * 	//There was an error
 * 	return -1;
 * }
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SETOC _IOCTLW(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 1, mcu_channel_t)

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
#define I_TMR_GETOC _IOCTLR(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 2, mcu_channel_t)

/*! \brief See details below.
 * \details This request writes the value of the input
 * capture using the current channel.  The ctl argument
 * is a pointer to the mcu_channel_t structure.
 * For example:
 *  * For example:
 *  * \code
 * mcu_channel_t chan_req;
 * chan_req.channel = 1; //write to OC channel 1
 * chan_req.value = 1; //write 1 to the IC channel
 * if ( ioctl(tmr_fd, I_TMR_SETIC, &chan_req ){
 * 	//There was an error
 * 	return -1;
 * }
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SETIC _IOCTLW(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 3, mcu_channel_t)

/*! \brief See details below.
 * \details This request reads the value of the input
 * capture using the current channel.  The ctl argument
 * is a pointer to the mcu_channel_t structure. For example:
 * \code
 * mcu_channel_t chan_req;
 * int ic_value;
 * chan_req.channel = 1;
 * if ( ioctl(tmr_fd, I_TMR_GETIC, &chan_req ){
 * 	//There was an error
 * 	return -1;
 * }
 * ic_value = chan_req.value;
 * \endcode
 * \hideinitializer
 */
#define I_TMR_GETIC _IOCTLR(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 4, mcu_channel_t)

/*! \brief See details below.
 * \details This request writes the value of the timer.
 * For example:
 * \code
 * u32 value = 1000000;
 * ioctl(tmr_fd, I_TMR_SET, value);
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SET _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 5)

/*! \brief See details below.
 * \details This request reads the value of the timer.
 * For example:
 * \code
 * u32 value;
 * value = ioctl(tmr_fd, I_TMR_GET);
 * \endcode
 * \hideinitializer
 */
#define I_TMR_GET _IOCTL(TMR_IOC_IDENT_CHAR, I_MCU_TOTAL + 6)

#define I_TMR_TOTAL 7

#ifdef __cplusplus
}
#endif

#endif // SOS_DEV_TMR_H_

/*! @} */

