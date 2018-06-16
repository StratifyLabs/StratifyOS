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

#define USB_VERSION (0x030001)
#define USB_IOC_IDENT_CHAR 'u'


/*! \details This lists the valid USB modes.
 *
 */
typedef enum {
	USB_FLAG_SET_UNCONFIGURED /*! unconfigured mode */ = (1<<0),
	USB_FLAG_SET_DEVICE /*! device mode */ = (1<<1),
	USB_FLAG_SET_HOST /*! host mode */ = (1<<2),
	USB_FLAG_SET_OTG /*! on-the-go mode */ = (1<<3),
	USB_FLAG_RESET = (1<<4),
	USB_FLAG_ATTACH = (1<<5),
	USB_FLAG_DETACH = (1<<6),
	USB_FLAG_CONFIGURE = (1<<7),
	USB_FLAG_UNCONFIGURE = (1<<8),
	USB_FLAG_SET_ADDRESS = (1<<9),
	USB_FLAG_RESET_ENDPOINT = (1<<10),
	USB_FLAG_ENABLE_ENDPOINT = (1<<11),
	USB_FLAG_DISABLE_ENDPOINT = (1<<12),
	USB_FLAG_STALL_ENDPOINT = (1<<13),
	USB_FLAG_UNSTALL_ENDPOINT = (1<<14),
	USB_FLAG_CONFIGURE_ENDPOINT = (1<<15),
	USB_FLAG_IS_SOF_ENABLED = (1<<16),
	USB_FLAG_IS_LOW_POWER_MODE_ENABLED = (1<<17),
	USB_FLAG_IS_VBUS_SENSING_ENABLED = (1<<18),
    USB_FLAG_IS_HIGH_SPEED = (1<<19)
} usb_flag_t;


typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
	u32 resd[8];
} usb_info_t;

#define USB_PIN_ASSIGNMENT_COUNT 4

typedef struct MCU_PACK {
	mcu_pin_t dp;
	mcu_pin_t dm;
	mcu_pin_t id;
	mcu_pin_t vbus;
} usb_pin_assignment_t;

#define USB_TX_FIFO_WORD_SIZE_COUNT 6

/*! \brief USB Attribute Data Structure
 * \details This defines the USB IOCTL data structure.
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Configuration flags */;
	usb_pin_assignment_t pin_assignment /*! Pin assignments */;
	u32 freq /*! The crystal oscillator frequency */;
	u32 address /*! USB endpoint address or device address (USB_FLAG_SET_ADDRESS) */;
	u16 max_packet_size /*! USB endpoing max packet size */;
    u16 type /*! USB endpoint type as bmAttributes */;
    u16 rx_fifo_word_size /*! RX FIFO word size for all endpoints (STM32) */;
    u8 tx_fifo_word_size[USB_TX_FIFO_WORD_SIZE_COUNT] /*! TX FIFO word size (used on STM32) */;
    u32 resd[6];
} usb_attr_t;

#define I_USB_GETVERSION _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_USB_GETINFO _IOCTLR(USB_IOC_IDENT_CHAR, I_MCU_GETINFO, usb_attr_t)
#define I_USB_SETATTR _IOCTLW(USB_IOC_IDENT_CHAR, I_MCU_SETATTR, usb_attr_t)
#define I_USB_SETACTION _IOCTLW(USB_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

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
#define I_USB_ISCONNECTED _IOCTL(USB_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)
#define I_USB_CONNECTED I_USB_ISCONNECTED

#define I_USB_TOTAL 1

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_USB_H_ */

/*! @} */
