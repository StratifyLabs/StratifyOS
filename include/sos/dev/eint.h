// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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
 * int configure_eint(){
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
 *		action.o_events = EINT_ACTION_EVENT_FALLING; //no events are defined for the EINT
 *		action.handler.callback = my_eint_action;
 *		action.handler.context = NULL;
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

#ifndef SOS_DEV_EINT_H_
#define SOS_DEV_EINT_H_

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EINT_VERSION (0x030000)
#define EINT_IOC_IDENT_CHAR 'e'

enum {
	EINT_FLAG_NONE = 0,
	EINT_FLAG_SET_INPUT = (1<<0)
};

typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask of supported attributes */;
	u32 value /*! Value of the pin (zero or non-zero) */;
	u32 resd[8];
} eint_info_t;


typedef struct MCU_PACK {
	mcu_pin_t channel[4];
	u32 resd[8];
} eint_pin_assignment_t;

/*! \brief External Interrupt Attribute Data Structure
 * \details This data structure is used for
 * controlling external interrupt IO peripherals.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Set the attibutes (use PIO_FLAG values) */;
	eint_pin_assignment_t pin_assignment;
} eint_attr_t;


#define I_EINT_GETVERSION _IOCTL(EINT_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief IOCTL request to get info.
 */
#define I_EINT_GETINFO _IOCTLR(EINT_IOC_IDENT_CHAR, I_MCU_GETINFO, eint_info_t)

/*! \brief IOCTL request to set the attributes (set pin assignment)
 */
#define I_EINT_SETATTR _IOCTLW(EINT_IOC_IDENT_CHAR, I_MCU_SETATTR, eint_attr_t)

/*! \brief See below for details.
 * \details This sets the action for an EINT channel.  For example:
 * \code
 * mcu_action_t action;
 * action.channel = 0; //channel if used on the architecture
 * action.handler.callback = my_callback; //function to execute when event happens
 * action.handler.context = NULL;  //passed to action.handler.callback as first argument
 * action.o_events = EINT_ACTION_EVENT_RISING; //Trigger on rising edge
 * ioctl(eint_fd, I_EINT_SETACTION, &action);
 * \endcode
 */
#define I_EINT_SETACTION _IOCTLW(EINT_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

#define I_EINT_TOTAL 0

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_EINT_H_ */

/*! @} */

