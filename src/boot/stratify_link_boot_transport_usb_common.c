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
#include "sos/link/link.h"
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "mcu/usbfifo.h"
#include "mcu/usbd/control.h"
#include "mcu/usbd/cdc.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "mcu/sys.h"

#include "sos/stratify_link_transport_usb.h"

#define STRATIFY_USBD_VID 0x20A0
#define STRATIFY_USBD_PID 0x41D5

#define STRATIFY_REQUIRED_CURRENT 500

#define STRATIFY_VCP0_INTERFACE 0
#define STRATIFY_VCP0_DATA_INTERFACE 1


#define USB0_DEVFIFO_BUFFER_SIZE 64
static char usb0_fifo_buffer[USB0_DEVFIFO_BUFFER_SIZE] MCU_SYS_MEM;
const usbfifo_cfg_t stratify_link_boot_transport_usb_fifo_cfg = USBFIFO_DEVICE_CFG(0,
		STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT,
		STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
		usb0_fifo_buffer,
		USB0_DEVFIFO_BUFFER_SIZE);
usbfifo_state_t stratify_link_boot_transport_usb_fifo_state MCU_SYS_MEM;


const usbd_control_constants_t stratify_link_boot_transport_usb_constants = {
		.port = STRATIFY_LINK_TRANSPORT_USB_PORT,
		.device =  &stratify_link_boot_transport_usb_dev_desc,
		.config = &stratify_link_boot_transport_usb_cfg_desc,
		.string = &stratify_link_boot_transport_usb_string_desc,
		.feature_event = 0,
		.configure_event = 0,
		.interface_event = 0,
		.adc_if_req = 0,
		.msc_if_req = 0,
		.cdc_if_req = stratify_link_transport_usbd_cdc_if_req,
		.hid_if_req = 0
};


/*! \details This variable stores the device descriptor.  It has a weak attribute and can be
 * overridden by using a user specific value
 * in the file \a devices.c.  This allows the user to change the USB vendor and product IDs.
 * All other values should be unchanged.
 *
 */
const usbd_device_descriptor_t stratify_link_boot_transport_usb_dev_desc MCU_WEAK = {
		.bLength = sizeof(usbd_device_descriptor_t),
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_DEVICE,
		.bcdUSB = 0x0200,
		.bDeviceClass = USBD_DEVICE_CLASS_COMMUNICATIONS,
		.bDeviceSubClass = 0,
		.bDeviceProtocol = 0,
		.bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
		.idVendor = STRATIFY_USBD_VID,
		.idProduct = STRATIFY_USBD_PID,
		.bcdDevice = 0x260,
		.iManufacturer = 1,
		.iProduct = 2,
		.iSerialNumber = 3,
		.bNumConfigurations = 1
};

const stratify_link_boot_transport_usbd_configuration_descriptor_t stratify_link_boot_transport_usb_cfg_desc MCU_WEAK = {

		.cfg = {
				.bLength = sizeof(usbd_configuration_descriptor_t),
				.bDescriptorType = USBD_DESCRIPTOR_TYPE_CONFIGURATION,

				.wTotalLength = sizeof(stratify_link_boot_transport_usbd_configuration_descriptor_t)-1, //exclude the zero terminator
				.bNumInterfaces = 0x02,
				.bConfigurationValue = 0x01,
				.iConfiguration = 0x03,
				.bmAttributes = USBD_CONFIGURATION_ATTRIBUTES_BUS_POWERED,
				.bMaxPower = USBD_CONFIGURATION_MAX_POWER_MA( STRATIFY_REQUIRED_CURRENT )
		},

		.vcp0 = {

				.interface_association = {
						.bLength = sizeof(usbd_interface_assocation_t),
						.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
						.bFirstInterface = STRATIFY_VCP0_INTERFACE,
						.bInterfaceCount = 2,
						.bFunctionClass = USBD_INTERFACE_CLASS_COMMUNICATIONS,
						.bFunctionSubClass = USBD_CDC_INTERFACE_SUBCLASS_ACM,
						.bFunctionProtocol = USBD_CDC_INTERFACE_PROTOCOL_V25TER,
						.iFunction = 0x04,
				},

				.interface_control = {
						.bLength = sizeof(usbd_interface_descriptor_t),
						.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE,
						.bInterfaceNumber = STRATIFY_VCP0_INTERFACE,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x01,
						.bInterfaceClass = USBD_INTERFACE_CLASS_COMMUNICATIONS,
						.bInterfaceSubClass = USBD_CDC_INTERFACE_SUBCLASS_ACM,
						.bInterfaceProtocol = USBD_CDC_INTERFACE_PROTOCOL_V25TER,
						.iInterface = 0x04
				},

				.acm = {
						.header.bFunctionLength = sizeof(usbd_cdc_header_functional_descriptor_t),
						.header.bDescriptorType = 0x24,
						.header.bDescriptorSubType = 0x00,
						.header.bcdCDC = 0x0110,
						.acm.bFunctionLength = sizeof(usbd_cdc_abstract_control_model_functional_descriptor_t),
						.acm.bDescriptorType = 0x24,
						.acm.bDescriptorSubType = 0x02, //ACM descriptor subtype
						.acm.bmCapabilities = 0x06, //support for SERIAL_STATE
						.union_descriptor.bFunctionLength = sizeof(usbd_cdc_union_functional_descriptor_t),
						.union_descriptor.bDescriptorType = 0x24,
						.union_descriptor.bDescriptorSubType = 0x06, //union descriptor subtype
						.union_descriptor.bMasterInterface = 0x00, //control interface
						.union_descriptor.bSlaveInterface = 0x01, //data interface
						.call_management.bFunctionLength = sizeof(usbd_cdc_call_management_functional_descriptor_t),
						.call_management.bDescriptorType = 0x24,
						.call_management.bDescriptorSubType = 0x01, //call management subtype
						.call_management.bmCapabilities = 0x03, //call management handled
						.call_management.bDataInterface = 0x01 //data interface
				},

				.control = {
						.bLength= sizeof(usbd_endpoint_descriptor_t),
						.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT,
						.bEndpointAddress=STRATIFY_LINK_TRANSPORT_USB_INTIN,
						.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_INTERRUPT,
						.wMaxPacketSize=16,
						.bInterval=2
				},

				.interface_data = {
						.bLength = sizeof(usbd_interface_descriptor_t),
						.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE,
						.bInterfaceNumber = STRATIFY_VCP0_DATA_INTERFACE,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x02,
						.bInterfaceClass = USBD_INTERFACE_CLASS_COMMUNICATIONS_DATA,
						.bInterfaceSubClass = USBD_CDC_INTERFACE_SUBCLASS_ACM,
						.bInterfaceProtocol = USBD_CDC_INTERFACE_PROTOCOL_V25TER,
						.iInterface = 0x04
				},

				.data_out = {
						.bLength= sizeof(usbd_endpoint_descriptor_t),
						.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT,
						.bEndpointAddress=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_OUT,
						.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK,
						.wMaxPacketSize=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
						.bInterval=1
				},

				.data_in = {
						.bLength= sizeof(usbd_endpoint_descriptor_t),
						.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT,
						.bEndpointAddress=STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN,
						.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK,
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
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING,
		.wLANGID = 0x0409, //English
		.manufacturer = usbd_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING),
		.product = usbd_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING),
		.serial = usbd_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE, 0)
		, //dynamically load SN based on silicon
		.vcp0 = usbd_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_0),
		.vcp1 = usbd_assign_string(STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE, STRATIFY_LINK_TRANSPORT_USB_DESC_VCP_1)
};
