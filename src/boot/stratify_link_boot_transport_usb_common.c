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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "iface/link.h"
#include "mcu/mcu.h"
#include "iface/dev/usb.h"
#include "dev/usbfifo.h"
#include "stratify/usb_dev.h"
#include "stratify/usb_dev_cdc.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "stratify/usb_dev_typedefs.h"
#include "stratify/usb_dev_defs.h"
#include "dev/sys.h"

#include "iface/stratify_link_transport_usb.h"

#define LINK_USB_VID 0x20A0
#define LINK_USB_PID 0x41D5

#ifndef LINK_USB_DEV_PORT
#define LINK_USB_DEV_PORT 0
#endif

#define LINK_REQD_CURRENT 500


#define USB0_DEVFIFO_BUFFER_SIZE 64
static char usb0_fifo_buffer[USB0_DEVFIFO_BUFFER_SIZE] MCU_SYS_MEM;
const usbfifo_cfg_t stratify_link_boot_transport_usb_fifo_cfg = USBFIFO_DEVICE_CFG(0,
		STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT,
		STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
		usb0_fifo_buffer,
		USB0_DEVFIFO_BUFFER_SIZE);
usbfifo_state_t stratify_link_boot_transport_usb_fifo_state MCU_SYS_MEM;


const usb_dev_const_t stratify_link_boot_transport_usb_constants = {
		.port = STRATIFY_LINK_TRANSPORT_USB_PORT,
		.device =  &stratify_link_boot_transport_usb_dev_desc,
		.config = &stratify_link_boot_transport_usb_cfg_desc,
		.string = &stratify_link_boot_transport_usb_string_desc,
		.feature_event = usb_dev_default_event,
		.configure_event = usb_dev_default_event,
		.interface_event = usb_dev_default_event,
		.adc_if_req = usb_dev_default_if_req,
		.msc_if_req = usb_dev_default_if_req,
		.cdc_if_req = stratify_link_transport_usb_cdc_if_req,
		.hid_if_req = usb_dev_default_if_req
};


/*! \details This variable stores the device descriptor.  It has a weak attribute and can be
 * overridden by using a user specific value
 * in the file \a devices.c.  This allows the user to change the USB vendor and product IDs.
 * All other values should be unchanged.
 *
 */
const usb_dev_desc_t stratify_link_boot_transport_usb_dev_desc MCU_WEAK = {
		.bLength = sizeof(usb_dev_desc_t),
		.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE,
		.bcdUSB = 0x0200,
		.bDeviceClass = USB_DEVICE_CLASS_COMMUNICATIONS,
		.bDeviceSubClass = USB_INTERFACE_SUBCLASS_ACM,
		.bDeviceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
		.bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
		.idVendor = LINK_USB_VID,
		.idProduct = LINK_USB_PID,
		.bcdDevice = 0x200,
		.iManufacturer = 1,
		.iProduct = 2,
		.iSerialNumber = 3,
		.bNumConfigurations = 1
};

const stratify_link_boot_transport_usb_cfg_desc_t stratify_link_boot_transport_usb_cfg_desc MCU_WEAK = {

		.cfg = {
				.bLength = sizeof(usb_cfg_desc_t),
				.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,

				.wTotalLength = sizeof(stratify_link_boot_transport_usb_cfg_desc_t)-1, //exclude the zero terminator
				.bNumInterfaces = 0x01,
				.bConfigurationValue = 0x01,
				.iConfiguration = 0x03,
				.bmAttributes = USB_CONFIG_BUS_POWERED,
				.bMaxPower = USB_CONFIG_POWER_MA( LINK_REQD_CURRENT )
		},

		.vcp0 = {

				.if_asso = {
						.bLength = sizeof(usb_dev_interface_assocation_t),
						.bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
						.bFirstInterface = 0,
						.bInterfaceCount = 2,
						.bFunctionClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
						.bFunctionSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bFunctionProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iFunction = 0x04,
				},

				.ifcontrol = {
						.bLength = sizeof(usb_interface_desc_t),
						.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
						.bInterfaceNumber = 0x00,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x01,
						.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
						.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iInterface = 0x04
				},

				.acm = {
						.header.bLength = sizeof(usb_dev_cdc_header_t),
						.header.bDescriptorType = 0x24,
						.header.bDescriptorSubType = 0x00,
						.header.bcdCDC = 0x0110,
						.acm.bFunctionLength = sizeof(usb_dev_cdc_acm_t),
						.acm.bDescriptorType = 0x24,
						.acm.bDescriptorSubType = 0x02, //ACM descriptor subtype
						.acm.bmCapabilities = 0x06, //support for SERIAL_STATE
						.union_descriptor.bFunctionLength = sizeof(usb_dev_cdc_uniondescriptor_t),
						.union_descriptor.bDescriptorType = 0x24,
						.union_descriptor.bDescriptorSubType = 0x06, //union descriptor subtype
						.union_descriptor.bMasterInterface = 0x00, //control interface
						.union_descriptor.bSlaveInterface = 0x01, //data interface
						.call_management.bFunctionLength = sizeof(usb_dev_cdc_callmanagement_t),
						.call_management.bDescriptorType = 0x24,
						.call_management.bDescriptorSubType = 0x01, //call management subtype
						.call_management.bmCapabilities = 0x03, //call management handled
						.call_management.bDataInterface = 0x01 //data interface
				},

				.control = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=STRATIFY_LINK_TRANSPORT_USB_INTIN,
						.bmAttributes=USB_ENDPOINT_TYPE_INTERRUPT,
						.wMaxPacketSize=16,
						.bInterval=2
				},

				.ifdata = {
						.bLength = sizeof(usb_interface_desc_t),
						.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
						.bInterfaceNumber = 0x01,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x02,
						.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS_DATA,
						.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iInterface = 0x04
				},

				.data_out = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_OUT,
						.bmAttributes=USB_ENDPOINT_TYPE_BULK,
						.wMaxPacketSize=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
						.bInterval=1
				},

				.data_in = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN,
						.bmAttributes=USB_ENDPOINT_TYPE_BULK,
						.wMaxPacketSize=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
						.bInterval=1
				}
		},


		.terminator = 0
};

/*! \details This variable stores the USB strings as the defaults listed below:
 * - Manufacturer: "Stratify OS, Inc"
 * - Product: "Stratify OS"
 * - Serial Number: "00000000"
 *
 * This variable has a weak attribute.  It can be overridden by using a user specific value
 * is the file \a devices.c.
 *
 */
const struct stratify_link_transport_usb_string_t stratify_link_boot_transport_usb_string_desc MCU_WEAK = {
		.bLength = 4,
		.bDescriptorType = USB_STRING_DESCRIPTOR_TYPE,
		.wLANGID = 0x0409, //English
		.manufacturer = usb_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING),
		.product = usb_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING),
		.serial = usb_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE, 0)
		, //dynamically load SN based on silicon
		.vcp0 = usb_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_0),
		.vcp1 = usb_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_1)
};
