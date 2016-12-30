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

/*! \addtogroup USBDEFS USB Definitions
 *
 *
 * @{
 *
 * \ingroup USB_DEV
 *
 */

#ifndef USB_TYPEDEFS_H_
#define USB_TYPEDEFS_H_

#include "mcu/types.h"

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
} usb_req_t;

typedef struct MCU_PACK {
	usb_req_t bmRequestType;
	u8 bRequest;
	u16_union_t wValue;
	u16_union_t wIndex;
	u16 wLength;
} usb_setup_pkt_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
} usb_ep_desc_t;

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
} usb_interface_desc_t;

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
} usb_dev_desc_t;

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
} usb_dev_qual_desc_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
} usb_cfg_desc_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u16 * bString;
	void * next;
} usb_string_desc_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
}usb_common_desc_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 bcdCDC;
} usb_dev_cdc_header_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usb_dev_cdc_acm_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bMasterInterface;
	u8 bSlaveInterface;
} usb_dev_cdc_uniondescriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
	u8 bDataInterface;
} usb_dev_cdc_callmanagement_t;


typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEntityId;
	u8 bExtensionCode;
	u8 iName;
	u8 bChildId[];
} usb_dev_cdc_extensionunit_t;

typedef struct MCU_PACK {
	usb_dev_cdc_header_t header;
	usb_dev_cdc_acm_t acm;
	usb_dev_cdc_uniondescriptor_t union_descriptor;
	usb_dev_cdc_callmanagement_t call_management;
} usb_cdc_acm_interface_t;

typedef struct MCU_PACK {
	usb_dev_cdc_header_t header;
	usb_dev_cdc_acm_t acm;
	usb_dev_cdc_callmanagement_t call_management;
} usb_cdc_acm_interface_alt_t;

typedef struct MCU_PACK {
	u8 bmRequestType;
	u8 bNotification;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} usb_cdc_acm_notification_t;

typedef struct MCU_PACK {
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
	u16 serial_state;
} usb_cdc_acm_notification_serialstate_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bFirstInterface;
	u8 bInterfaceCount;
	u8 bFunctionClass;
	u8 bFunctionSubClass;
	u8 bFunctionProtocol;
	u8 iFunction;
} usb_dev_interface_assocation_t;

typedef struct MCU_PACK {
	usb_dev_interface_assocation_t if_asso;
	usb_interface_desc_t ifcontrol /* The interface descriptor */;
	usb_cdc_acm_interface_t acm /*! The CDC ACM Class descriptor */;
	usb_ep_desc_t control /* Endpoint:  Interrupt out for control packets */;
	usb_interface_desc_t ifdata /* The interface descriptor */;
	usb_ep_desc_t data_out /* Endpoint:  Bulk out */;
	usb_ep_desc_t data_in /* Endpoint:  Bulk in */;
} usb_desc_vcp_t;

typedef struct MCU_PACK {
	u16 bcdDevice;
	u16 idProduct;
	u16 idVendor;
	u16 bcdDFU;
	char ucDfuSignature[3];
	u8 bLength;
	uint32_t dwCRC;
} usb_dev_dfu_file_suffix_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bmAttributes;
	u16 wDetachTimeOut;
	u16 wTransferSize;
	u16 bcdDFUVersion;
} usb_dev_dfu_func_desc_t;

typedef struct MCU_PACK {
	u8 bStatus;
	u8 bwPollTimeout[3];
	u8 bState;
	u8 iString;
} usb_dev_dfu_status_t;



/*! \details This macro function allows the user to declare a USB string
 * data structure.
 */
#define usb_declare_string(len) struct \
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
#define usb_assign_string(len, ...) { \
		.bLength = len*2+2, \
		.bDescriptorType = USB_STRING_DESCRIPTOR_TYPE, \
		.string = { __VA_ARGS__ } }


#endif /* USB_TYPEDEFS_H_ */

/*! @} */
