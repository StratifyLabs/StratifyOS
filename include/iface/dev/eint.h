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

/*! \addtogroup EINT External Interrupts (EINT)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The External Interrupt API allows the user to configure how
 * the external interrupts pins trigger IRQs.
 *
 * The following is an example of how to use the EINT in an OS environment:
 *
 * \code
 *
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <errno.h>
 * #include <stdio.h>
 * #include "mcu/mcu.h"
 *
 * void my_eint_action(void * context, uint32_t value){
 * 	//take some action on an eint event -- this runs in priveleged, handler mode, it should be fast and short
 * }
 *
 * int configure_eint(void){
 * 	int fd;
 * 	eint_attr_t attr;
 * 	eint_action_t action;
 *
 * 	fd = open("/dev/eint0", O_RDWR);
 * 	if ( fd < 0 ){
 * 		printf("Error opening peripheral (%d)\n", errno);
 * 	} else {
 *		attr.pin_assign = 0;
 * 		if( ioctl(fd, I_SETATTR, &ctl) < 0 ){
 * 			printf("Failed to set peripheral configuration (%d)\n", errno);
 * 			return -1;
 *		}
 *
 *		//Now set an action to take when a falling edge occurs on eint0
 *		action.channel = 0; //unused
 *		action.event = EINT_ACTION_EVENT_FALLING; //no events are defined for the EINT
 *		action.callback = my_eint_action;
 *		action.context = NULL;
 *		if ( ioctl(fd, I_SETACTION, &action) ){
 *			printf("Error setting action for peripheral (%d)\n", errno);
 *			return -1;
 *		}
 *
 * 	}
 * 	close(fd);
 * 	return 0;
 * }
 *
 * \endcode
 *
 */

/*! \file
 * \brief External Interrupt Header File
 *
 */

#ifndef DEV_EINT_H_
#define DEV_EINT_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"
#include "mcu/pio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EINT_IOC_IDENT_CHAR 'e'

/*! \brief Lists the valid values for the external interrupt trigger.
 * \details This list defines the valid values for the external interrupt
 * trigger value.
 * \return An HWPL Error code
 * \sa ERR
 */
typedef enum {
	EINT_ACTION_EVENT_UNCONFIGURED /*! The trigger has not been set */,
	EINT_ACTION_EVENT_RISING /*! Trigger on the rising edge */,
	EINT_ACTION_EVENT_FALLING /*! Trigger on the falling edge */,
	EINT_ACTION_EVENT_BOTH /*! Trigger on both edges */,
	EINT_ACTION_EVENT_LOW /*! Trigger while low */,
	EINT_ACTION_EVENT_HIGH /*! Trigger while high */
} eint_action_event_t;

/*! \brief See below for details.
 * \details This defines a EINT action.
 *
 * See \ref eint_action_event_t for events.  There
 * are no channel specific definitions for the EINT.
 *
 */
typedef mcu_action_t eint_action_t;

/*! \brief External Interrupt Attribute Data Structure
 * \details This data structure is used for
 * controlling external interrupt IO peripherals.
 */
typedef struct MCU_PACK {
	uint8_t pin_assign /*! \brief The Pin assignment to use (see \ref LPC17XXDEV) */;
	uint8_t value /*! \brief Value of input at pin (1 for high and zero for low) */;
	uint16_t mode /*! \brief Pin mode for pulling up/down/float */;
} eint_attr_t;

/*! \brief IOCTL request to get attributes.
 */
#define I_EINT_GETATTR _IOCTLR(EINT_IOC_IDENT_CHAR, I_GLOBAL_GETATTR, eint_attr_t)

/*! \brief IOCTL request to set the attributes (set pin assignment)
 */
#define I_EINT_SETATTR _IOCTLW(EINT_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, eint_attr_t)

/*! \brief See below for details.
 * \details This sets the action for an EINT channel.  For example:
 * \code
 * eint_action_t action;
 * action.channel = 0; //channel if used on the architecture
 * action.callback = my_callback; //function to execute when event happens
 * action.context = NULL;  //passed to my_callback as first argument
 * action.event = EINT_ACTION_EVENT_RISING; //Trigger on rising edge
 * ioctl(eint_fd, I_EINT_SETACTION, &action);
 * \endcode
 */
#define I_EINT_SETACTION _IOCTLW(EINT_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, eint_action_t)

#define I_EINT_TOTAL 0

#ifdef __cplusplus
}
#endif

#endif /* DEV_EINT_H_ */

/*! @} */

