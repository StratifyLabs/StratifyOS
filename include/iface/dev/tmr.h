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
 * int my_tmr_action(void * context, uint32_t value){
 * 	//take some action on an eint event -- this runs in priveleged, handler mode, it should be fast and short
 * 	return 1; //return non-zero to keep the action between successive events
 * }
 *
 * int tmr_example(void){
 * 	int fd;
 * 	tmr_attr_t attr;
 * 	tmr_action_t action;
 * 	tmr_reqattr_t chan_req;
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
 *		action.event = TMR_ACTION_EVENT_RESET;
 *		action.callback = NULL;
 *		action.context = NULL;
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

#ifndef DEV_TMR_H_
#define DEV_TMR_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TMR_IOC_IDENT_CHAR 't'

#ifndef TMR_SAMPLE_T
#define TMR_SAMPLE_T
typedef uint32_t tmr_sample_t;
#endif

/*! \details This lists the valid values for the
 * clk_src member of the tmr_attr_t data structure.
 */
typedef enum {
	TMR_CLKSRC_CPU /*! Use the CPU as the source for the clock (timer mode) */ = 0,
	TMR_CLKSRC_IC0 /*! Use input capture channel 0 for the clock source (counter mode) */ = 1,
	TMR_CLKSRC_IC1 /*! Use input capture channel 1 for the clock source (counter mode) */ = 2,
	TMR_CLKSRC_IC2 /*! Use input capture channel 2 for the clock source (counter mode) */ = 3,
	TMR_CLKSRC_IC3 /*! Use input capture channel 3 for the clock source (counter mode) */ = 4,
	TMR_CLKSRC_EDGERISING /*! Count rising edges */ = (1<<4),
	TMR_CLKSRC_EDGEFALLING /*! Count falling edges */ = (2<<4),
	TMR_CLKSRC_EDGEBOTH /*! Count both edges */ = (3<<4),
	TMR_CLKSRC_COUNTDOWN /*! Count down (not up) */ = (1<<8)
} tmr_clksrc_t;

#define TMR_CLKSRC_MASK (0x0F)
#define TMR_CLKSRC_EDGEMASK (0xF0)




/*! \details This defines no action
 * for an OC/IC event.
 */
typedef enum {
	TMR_ACTION_EVENT_NONE /*! no action */ = 0,
	TMR_ACTION_EVENT_INTERRUPT /*! enable interrupt on IC/OC event */ = (1<<0),
	TMR_ACTION_EVENT_RESET /*! reset the timer on IC/OC event */ = (1<<1),
	TMR_ACTION_EVENT_STOP /*! stop the timer on OC/IC event */ = (1<<2),
	TMR_ACTION_EVENT_SETOC /*! set the OC channel pin to logic 1 */ = (1<<3),
	TMR_ACTION_EVENT_CLROC /*! clear the OC channel pin to logic 0 */ = (1<<4),
	TMR_ACTION_EVENT_TOGGLEOC /*! toggle the OC channel pin */ = (1<<5),
	TMR_ACTION_EVENT_PWMMODE /*! Execute OC channel in PWM Mode */ = (1<<6),
} tmr_action_event_t;


/*! \details This defines the timer action structure.
 *
 * See \ref tmr_action_event_t for event values.
 *
 * See the following for channel information:
 * - \ref lpc17xx_tmr_action_channel_t
 * - \ref lpc13xx_tmr_action_channel_t
 *
 * for action channel information.
 *
 * \sa mcu_action_t
 */
typedef mcu_action_t tmr_action_t;


/*! \brief A Timer Channel (OC or IC) Request Data
 * \details This structure is used when dealing
 * with output compare or input capture channels request using tmr_ioctl().
 */
typedef struct MCU_PACK {
	u32 channel /*! \brief The OC or IC channel */;
	u32 value /*! \brief The value to read or write */;
} tmr_reqattr_t;


/*! \brief Specifies the timer clock source as part of the mode.
 * \hideinitializer
 */
#define TMR_MODE_SETCLKSRC(x) ( x & 0x0F )
#define TMR_MODE_CLK_SRC(x) TMR_MODE_SETCLKSRC

/*! \brief Is used to specify the timer counter mode when opening a timer port.
 * \hideinitializer
 *
 */
#define TMR_MODE_SETCOUNTER(x) ( (x&0x03)<<4 )
#define TMR_MODE_COUNTER(x) TMR_MODE_SETCOUNTER(x)


#define TMR_MODE_GET_CLK_SRC(mode) ( mode & 0x0F )
#define TMR_MODE_GET_COUNTER(mode) ( (mode >> 4) & 0x03 )

/*! \brief Timer Attribute Data Structure
 * \details This structure defines how the control structure
 * for opening or reconfiguring a timer port.
 */
typedef struct MCU_PACK {
	uint8_t pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	uint32_t freq /*! \brief The target clock frequency when writing and the actual when reading (only applicable to \ref TMR_CLKSRC_CPU) */;
	uint8_t clksrc /*! \brief The clock source (see \ref tmr_clksrc_t) */;
	uint8_t counter_mode /*! \brief Deprecated */;
	uint8_t enabled_oc_chans /*! \brief The mask of enabled output compare channels.  Enabled
	channels that do not exist are ignored. These are only set if the output compare units will
	use GPIO. */;
	uint8_t enabled_ic_chans /*! \brief The mask of enabled input capture channels.  Enabled
	channels that do not exist are ignored. These are only set if the input capture units will
	use GPIO. */;
} tmr_attr_t;

/*! \brief This request gets the timer attributes.
 * \hideinitializer
 *
 */
#define I_TMR_ATTR _IOCTLR(TMR_IOC_IDENT_CHAR, I_GLOBAL_ATTR, tmr_attr_t)
#define I_TMR_GETATTR I_TMR_ATTR
/*! \brief This request sets the timer attributes.
 * \hideinitializer
 */
#define I_TMR_SETATTR _IOCTLW(TMR_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, tmr_attr_t)
/*! \brief This request sets the timer interrupt action.
 * \hideinitializer
 */
#define I_TMR_SETACTION _IOCTLW(TMR_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, tmr_action_t)

/*! \brief This request turns the timer on.  After
 * a timer is opened it must be turned on before it
 * starts running.  The ctl argument is NULL.
 * \hideinitializer
 */
#define I_TMR_ON _IOCTL(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0)

/*! \brief This request turns the timer off.  After
 * a timer is opened it is off.
 * \hideinitializer
 */
#define I_TMR_OFF _IOCTL(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)

/*! \brief See details below.
 * \details This request writes the value of the
 * current output compare channel. The ctl
 * argument is a pointer to the tmr_reqattr_t structure.
 * For example:
 *  * \code
 * tmr_reqattr_t chan_req;
 * chan_req.channel = 1; //write to OC channel 1
 * chan_req.value = 1000000; //write 1 million to the OC channel
 * if ( ioctl(tmr_fd, I_TMR_SETOC, &chan_req ){
 * 	//There was an error
 * 	return -1;
 * }
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SETOC _IOCTLW(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2, tmr_reqattr_t)
#define I_TMR_SET_OC I_TMR_SETOC

/*! \brief See details below.
 * \details This request reads the value of the
 * current output compare channel. The ctl
 * argument is a pointer to a tmr_reqattr_t structure.  tmr_ioctl() will return
 * zero on success.  For example:
 * \code
 * tmr_reqattr_t chan_req;
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
#define I_TMR_GETOC _IOCTLR(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3, tmr_reqattr_t)
#define I_TMR_GET_OC I_TMR_GETOC

/*! \brief See details below.
 * \details This request writes the value of the input
 * capture using the current channel.  The ctl argument
 * is a pointer to the tmr_reqattr_t structure.
 * For example:
 *  * For example:
 *  * \code
 * tmr_reqattr_t chan_req;
 * chan_req.channel = 1; //write to OC channel 1
 * chan_req.value = 1; //write 1 to the IC channel
 * if ( ioctl(tmr_fd, I_TMR_SETIC, &chan_req ){
 * 	//There was an error
 * 	return -1;
 * }
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SETIC _IOCTLW(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 4, tmr_reqattr_t)
#define I_TMR_SET_IC I_TMR_SETIC

/*! \brief See details below.
 * \details This request reads the value of the input
 * capture using the current channel.  The ctl argument
 * is a pointer to the tmr_reqattr_t structure. For example:
 * \code
 * tmr_reqattr_t chan_req;
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
#define I_TMR_GETIC _IOCTLR(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 5, tmr_reqattr_t)
#define I_TMR_GET_IC I_TMR_GETIC

/*! \brief See details below.
 * \details This request writes the value of the timer.
 * For example:
 * \code
 * uint32_t value = 1000000;
 * ioctl(tmr_fd, I_TMR_SET, value);
 * \endcode
 * \hideinitializer
 */
#define I_TMR_SET _IOCTL(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 6)

/*! \brief See details below.
 * \details This request reads the value of the timer.
 * For example:
 * \code
 * uint32_t value;
 * value = ioctl(tmr_fd, I_TMR_GET);
 * \endcode
 * \hideinitializer
 */
#define I_TMR_GET _IOCTL(TMR_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 7)

#define I_TMR_TOTAL 8

#ifdef __cplusplus
}
#endif

#endif // DEV_TMR_H_

/*! @} */

