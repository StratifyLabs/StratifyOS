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

/*! \addtogroup USBDEFS USB Definitions
 *
 *
 * @{
 *
 * \ingroup USB_DEV
 *
 */

#ifndef USBD_TYPES_H_
#define USBD_TYPES_H_

#include "mcu/types.h"
#include "defines.h"

typedef union MCU_PACK {
	u16 w;
	u8 b[2];
} u16_union_t;

typedef union MCU_PACK {
	struct MCU_PACK {
		u8 recipient:5;
		u8 type:2;
		u8 dir:1;
	} bitmap_t;
	u8 b;
} usbd_request_t;

typedef struct MCU_PACK {
	usbd_request_t bmRequestType;
	u8 bRequest;
	u16_union_t wValue;
	u16_union_t wIndex;
	u16 wLength;
} usbd_setup_packet_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
} usbd_endpoint_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
} usbd_interface_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} usbd_device_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize;
	u8 bNumConfigurations;
	u8 bReserved;
} usbd_qualifier_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
} usbd_configuration_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u16 * bString;
	void * next;
} usbd_string_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
} usbd_common_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bFirstInterface;
	u8 bInterfaceCount;
	u8 bFunctionClass;
	u8 bFunctionSubClass;
	u8 bFunctionProtocol;
	u8 iFunction;
} usbd_interface_assocation_t;



/*! \details This macro function allows the user to declare a USB string
 * data structure.
 */
#define usbd_declare_string(len) struct \
		MCU_PACK { \
	u8 bLength; \
	u8 bDescriptorType; \
	u16 string[len]; \
}

/*! \details This macro function allows the user to assign values to a USB
 * string using comma separated characters.
 *
 * \param len The number of characters in the string.
 * \param ... Comma separated characters (e.g. 'E','x','a','m','p','l','e')
 */
#define usbd_assign_string(len, ...) { \
		.bLength = len*2+2, \
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING, \
		.string = { __VA_ARGS__ } }


#endif /* USBD_TYPES_H_ */

/*! @} */
