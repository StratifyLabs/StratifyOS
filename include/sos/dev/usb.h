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

/*! \addtogroup USB Universal Serial Bus (USB)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The HWPL USB module provides an API for accessing the devices USB hardware.  The full
 * USB device stack is part of the Hardware Device Library (HWDL).  The HWPL API is used for
 * low level initialization, and the HWDL is used for application level functionality such as implementing USB
 * Device Classes.
 *
 * More information about accessing peripheral IO is in the \ref IFACE_DEV section.
 *
 */

/*! \file
 * \brief Universal Serial Bus Header File
 *
 */

#ifndef SOS_DEV_USB_H_
#define SOS_DEV_USB_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_IOC_IDENT_CHAR 'u'


/*! \details This lists the valid USB modes.
 *
 */
typedef enum {
	USB_FLAG_SET_UNCONFIGURED /*! unconfigured mode */ = (1<<0),
	USB_FLAG_SET_DEVICE /*! device mode */ = (1<<1),
	USB_FLAG_SET_HOST /*! host mode */ = (1<<2),
	USB_FLAG_SET_OTG /*! on-the-go mode */ = (1<<3),
} usb_flag_t;


typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
} usb_info_t;

#define USB_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	mcu_pin_t dp;
	mcu_pin_t dm;
	mcu_pin_t id;
	mcu_pin_t vbus;
} usb_pin_assignment_t;

/*! \brief USB Attribute Data Structure
 * \details This defines the USB IOCTL data structure.
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Configuration flags */;
	usb_pin_assignment_t pin_assignment /*! Pin assignments */;
	u32 freq /*! \brief The crystal oscillator frequency */;
} usb_attr_t;

/*! \brief This request gets the USB attributes.
 * \hideinitializer
 */
#define I_USB_GETINFO _IOCTLR(USB_IOC_IDENT_CHAR, I_MCU_GETINFO, usb_attr_t)

/*! \brief This request sets the USB attributes.
 * \hideinitializer
 */
#define I_USB_SETATTR _IOCTLW(USB_IOC_IDENT_CHAR, I_MCU_SETATTR, usb_attr_t)
#define I_USB_SETACTION _IOCTLW(USB_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * usb_ioctl(usb_fd, I_USB_RESET);
 * \endcode
 * \hideinitializer
 */
#define I_USB_RESET _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)

/*! \brief See details below.
 * \details This request attaches the USB.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_ATTACH);
 * \endcode
 * \hideinitializer
 */
#define I_USB_ATTACH _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)

/*! \brief See details below.
 * \details This request detaches the USB
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_DETACH);
 * \endcode
 * \hideinitializer
 */
#define I_USB_DETACH _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)

/*! \brief See details below.
 * \details This request configures the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_CONFIGURE);
 * \endcode
 * \hideinitializer
 */
#define I_USB_CONFIGURE _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 3)

/*! \brief See details below.
 * \details This request sets the USB address.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_SETADDR, usb_addr);
 * \endcode
 * \hideinitializer
 */
#define I_USB_SETADDR _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 4)

/*! \brief See details below.
 * \details This request resets the specified USB endpoint.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_RESETEP, endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_RESETEP _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 5)
#define I_USB_RESET_EP I_USB_RESETEP

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_ENABLEEP, endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_ENABLEEP _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 6)
#define I_USB_ENABLE_EP I_USB_ENABLEEP

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_DISABLEEP, endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_DISABLEEP _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 7)
#define I_USB_DISABLE_EP I_USB_DISABLEEP

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_STALL_EP, endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_STALLEP _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 8)
#define I_USB_STALL_EP I_USB_STALLEP

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_UNSTALLEP, (void*)endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_UNSTALLEP _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 9)
#define I_USB_UNSTALL_EP I_USB_UNSTALLEP

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_CFGEP, endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_CFGEP _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 10)
#define I_USB_CFG_EP I_USB_CFGEP


/*! \brief See details below.
 * \details This request sets the function to handle events.
 *
 * Example:
 * \code
 * void my_handler(usb_spec_event_t event);
 * if ( ioctl(usb_fd, I_USB_SETEVENTHANDLER, my_handler) ){
 * }
 * \endcode
 * \hideinitializer
 */
#define I_USB_SETEVENTHANDLER _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 11)
#define I_USB_SET_EVENT_HANDLER I_USB_SETEVENTHANDLER


/*! \brief See details below.
 * \details This request checks to see if the USB is connected to an upstream port.
 * It is connected, once is has been reset and disconnected when suspended.
 *
 *  * Example:
 * \code
 * void my_handler(usb_spec_event_t event);
 * if ( ioctl(usb_fd, I_USB_ISCONNECTED) ){
 *  //The USB is connected
 * } else {
 * 	//The USB is not connected
 * }
 * \endcode
 *
 * \hideinitializer
 */
#define I_USB_ISCONNECTED _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 12)
#define I_USB_CONNECTED I_USB_ISCONNECTED

#define I_USB_TOTAL 13

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_USB_H_ */

/*! @} */
