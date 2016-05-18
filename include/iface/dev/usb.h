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

#ifndef DEV_USB_H_
#define DEV_USB_H_


#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_IOC_IDENT_CHAR 'u'

#ifndef USB_LOGIC_EP_NUM
#define USB_LOGIC_EP_NUM 0
#endif

enum {
	USB_SETUP_EVENT /*! USB Setup event */,
	USB_OUT_EVENT /*! USB Out event (Out from host to device) */,
	USB_IN_EVENT /*! USB In event (In to host from device) */,
	USB_OUT_EVENT_STALL /*! Out stall event */,
	USB_IN_EVENT_STALL /*! In stall event */
};

/*! \details This defines a USB action.
 *
 */
typedef mcu_action_t usb_action_t;


/*! \details These are the special events that can be passed to
 * the event callback handler.
 */
typedef enum {
	USB_SPEC_EVENT_NONE /*! 0 */,
	USB_SPEC_EVENT_RESET /*! 1 Reset Event */,
	USB_SPEC_EVENT_POWER /*! 2 Power Event */,
	USB_SPEC_EVENT_SUSPEND /*! 3 Suspend Event */,
	USB_SPEC_EVENT_RESUME /*! 4 Resume Event */,
	USB_SPEC_EVENT_DEBUG /*! 5 Debug Event */,
	USB_SPEC_EVENT_WAKEUP /*! 6 Wakeup Event */,
	USB_SPEC_EVENT_WAKEUP_CFG /*! 7 Wakeup Configure Event */,
	USB_SPEC_EVENT_DIR_CTRL_EP /*! 8 Direction Control Event */,
	USB_SPEC_EVENT_ERR /*! 9 Error Event */,
	USB_SPEC_EVENT_FIQ /*! 10 Fast Interrupt Event */,
	USB_SPEC_EVENT_SOF /*! 11 Start of Frame Event */,
	USB_SPEC_EVENT_TOTAL /*! Total Number of Events */
} usb_spec_event_t;


/*! \details This lists the valid USB modes.
 *
 */
typedef enum {
	USB_ATTR_MODE_UNCONFIGURED /*! unconfigured mode */,
	USB_ATTR_MODE_DEVICE /* device mode */,
	USB_ATTR_MODE_HOST /*! host mode */,
	USB_ATTR_MODE_OTG /*! on-the-go mode */
} usb_attr_mode_t;


typedef enum {
	USB_EVENT_DATA_READY,
	USB_EVENT_WRITE_COMPLETE,
	USB_EVENT_TOTAL
} usb_action_event_t;

/*! \brief USB Attribute Data Structure
 * \details This defines the USB IOCTL data structure.
 *
 */
typedef struct MCU_PACK {
	u8 pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	u8 mode /*! \brief The mode see \ref usb_attr_mode_t */;
	u16 resd;
	u32 crystal_freq /*! \brief The crystal oscillator frequency */;
} usb_attr_t;

/*! \brief See details below.
 * \details This defines the type for a USB event handler.
 *
 */
typedef void (*usb_event_handler_t)(usb_spec_event_t);

/*! \brief This request gets the USB attributes.
 * \hideinitializer
 */
#define I_USB_ATTR _IOCTLR(USB_IOC_IDENT_CHAR, I_GLOBAL_ATTR, usb_attr_t)
#define I_USB_GETATTR I_USB_ATTR

/*! \brief This request sets the USB attributes.
 * \hideinitializer
 */
#define I_USB_SETATTR _IOCTLW(USB_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, usb_attr_t)
#define I_USB_SETACTION _IOCTLW(USB_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, usb_action_t)

/*! \brief See details below.
 * \details This request resets the USB interface.
 *
 * Example:
 * \code
 * usb_ioctl(usb_fd, I_USB_RESET);
 * \endcode
 * \hideinitializer
 */
#define I_USB_RESET _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0)

/*! \brief See details below.
 * \details This request attaches the USB.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_ATTACH);
 * \endcode
 * \hideinitializer
 */
#define I_USB_ATTACH _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)

/*! \brief See details below.
 * \details This request detaches the USB
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_DETACH);
 * \endcode
 * \hideinitializer
 */
#define I_USB_DETACH _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2)

/*! \brief See details below.
 * \details This request configures the USB interface.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_CONFIGURE);
 * \endcode
 * \hideinitializer
 */
#define I_USB_CONFIGURE _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3)

/*! \brief See details below.
 * \details This request sets the USB address.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_SETADDR, usb_addr);
 * \endcode
 * \hideinitializer
 */
#define I_USB_SETADDR _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 4)

/*! \brief See details below.
 * \details This request resets the specified USB endpoint.
 *
 * Example:
 * \code
 * ioctl(usb_fd, I_USB_RESETEP, endpoint_number);
 * \endcode
 * \hideinitializer
 */
#define I_USB_RESETEP _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 5)
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
#define I_USB_ENABLEEP _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 6)
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
#define I_USB_DISABLEEP _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 7)
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
#define I_USB_STALLEP _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 8)
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
#define I_USB_UNSTALLEP _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 9)
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
#define I_USB_CFGEP _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 10)
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
#define I_USB_SETEVENTHANDLER _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 11)
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
#define I_USB_ISCONNECTED _IOCTL(USB_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 12)
#define I_USB_CONNECTED I_USB_ISCONNECTED

#define I_USB_TOTAL 13

#ifdef __cplusplus
}
#endif

#endif /* DEV_USB_H_ */

/*! @} */
