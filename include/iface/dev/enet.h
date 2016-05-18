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
 * \details The Ethernet interface
 *
 * \code
 *
 *
 * \endcode
 *
 */

/*! \file
 * \brief External Interrupt Header File
 *
 */

#ifndef DEV_ENET_H_
#define DEV_ENET_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENET_IOC_IDENT_CHAR 'E'

/*! \brief Lists the valid values for the external interrupt trigger.
 * \details This list defines the valid values for the external interrupt
 * trigger value.
 * \return An HWPL Error code
 * \sa ERR
 */
typedef enum {
	ENET_ACTION_EVENT_UNCONFIGURED /*! The trigger has not been set */,
	ENET_ACTION_EVENT_RISING /*! Trigger on the rising edge */,
	ENET_ACTION_EVENT_FALLING /*! Trigger on the falling edge */,
	ENET_ACTION_EVENT_BOTH /*! Trigger on both edges */,
	ENET_ACTION_EVENT_LOW /*! Trigger while low */,
	ENET_ACTION_EVENT_HIGH /*! Trigger while high */
} enet_action_event_t;


enum {
	ENET_FLAGS_USERMII /*! \brief Use RMII */ = (1<<0),
	ENET_FLAGS_FULLDUPLEX /*! \brief Use Full Duplex */ = (1<<1),
	ENET_FLAGS_HALFDUPLEX /*! \brief Use Half Duplex (default) */ = 0,
};


/*! \brief See below for details.
 * \details This defines a EINT action.
 *
 * See \ref enet_action_event_t for events.  There
 * are no channel specific definitions for the EINT.
 *
 */
typedef mcu_action_t enet_action_t;

/*! \brief External Interrupt Attribute Data Structure
 * \details This data structure is used for
 * controlling external interrupt IO peripherals.
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The Pin assignment to use (see \ref LPC17XXDEV) */;
	u8 resd;
	u16 flags /*! \brief Flag settings (see ENET_FLAGS_*) */;
	u8 mac_addr[6] /*! \brief MAC address */;
} enet_attr_t;

/*! \brief IOCTL request to get attributes.
 */
#define I_ENET_ATTR _IOCTLR(ENET_IOC_IDENT_CHAR, I_GLOBAL_ATTR, enet_attr_t)
#define I_ENET_GETATTR I_ENET_GETATTR

/*! \brief IOCTL request to set the attributes (set pin assignment)
 */
#define I_ENET_SETATTR _IOCTLW(ENET_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, enet_attr_t)

/*! \brief See below for details.
 * \details This sets the action for an EINT channel.  For example:
 * \code
 * enet_action_t action;
 * action.channel = 0; //channel if used on the architecture
 * action.callback = my_callback; //function to execute when event happens
 * action.context = NULL;  //passed to my_callback as first argument
 * action.event = ENET_ACTION_EVENT_RISING; //Trigger on rising edge
 * ioctl(enet_fd, I_ENET_SETACTION, &action);
 * \endcode
 */
#define I_ENET_SETACTION _IOCTLW(ENET_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, enet_action_t)

#define I_ENET_TOTAL 1

#ifdef __cplusplus
}
#endif

#endif /* DEV_ENET_H_ */

/*! @} */

