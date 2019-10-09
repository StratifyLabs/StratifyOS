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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "config.h"
#include "sos/link.h"
#include "mcu/mcu.h"
#include "mcu/boot_debug.h"
#include "sos/dev/usb.h"
#include "device/usbfifo.h"
#include "usbd/control.h"
#include "usbd/cdc.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "device/sys.h"

#include "sos/link/transport_usb.h"

#define SOS_USBD_VID 0x20A0
#define SOS_USBD_PID 0x41D5

#define SOS_REQUIRED_CURRENT 500

#define SOS_VCP0_INTERFACE 0
#define SOS_VCP0_DATA_INTERFACE 1

static int cdc_event_handler(usbd_control_t * context, const mcu_event_t * event);


const usbd_control_constants_t sos_link_transport_usb_constants = {
	.handle.port = SOS_LINK_TRANSPORT_USB_PORT,
	.handle.config = 0,
	.handle.state = 0,
	.device =  &sos_link_transport_usb_dev_desc,
	.config = &sos_link_transport_usb_cfg_desc,
	.qualifier = &sos_link_transfer_usb_qualifer_desc,
	.string = &sos_link_transport_usb_string_desc,
	.class_event_handler = sos_link_usbd_cdc_event_handler
};


/*! \details This variable stores the device descriptor.  It has a weak attribute and can be
 * overridden by using a user specific value
 * in the file \a devices.c.  This allows the user to change the USB vendor and product IDs.
 * All other values should be unchanged.
 *
 */
const usbd_device_descriptor_t sos_link_transport_usb_dev_desc MCU_WEAK = {
	.bLength = sizeof(usbd_device_descriptor_t),
	.bDescriptorType = USBD_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USBD_DEVICE_CLASS_COMMUNICATIONS,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
	.idVendor = SOS_USBD_VID,
	.idProduct = SOS_USBD_PID,
	.bcdDevice = BCD_VERSION,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1
};

const usbd_qualifier_descriptor_t sos_link_transfer_usb_qualifer_desc MCU_WEAK = {
	.bLength = sizeof(usbd_device_descriptor_t),
	.bDescriptorType = USBD_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USBD_DEVICE_CLASS_COMMUNICATIONS,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
	.bReserved = 0
};

const sos_link_transport_usb_configuration_descriptor_t sos_link_transport_usb_cfg_desc MCU_WEAK = {

	.cfg = {
		.bLength = sizeof(usbd_configuration_descriptor_t),
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_CONFIGURATION,

		.wTotalLength = sizeof(sos_link_transport_usb_configuration_descriptor_t)-1, //exclude the zero terminator
		.bNumInterfaces = 0x02,
		.bConfigurationValue = 0x01,
		.iConfiguration = 0x03,
		.bmAttributes = USBD_CONFIGURATION_ATTRIBUTES_BUS_POWERED,
		.bMaxPower = USBD_CONFIGURATION_MAX_POWER_MA( SOS_REQUIRED_CURRENT )
	},

	.vcp0 = {

		.interface_association = {
			.bLength = sizeof(usbd_interface_assocation_t),
			.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
			.bFirstInterface = SOS_VCP0_INTERFACE,
			.bInterfaceCount = 2,
			.bFunctionClass = USBD_INTERFACE_CLASS_COMMUNICATIONS,
			.bFunctionSubClass = USBD_CDC_INTERFACE_SUBCLASS_ACM,
			.bFunctionProtocol = USBD_CDC_INTERFACE_PROTOCOL_V25TER,
			.iFunction = 0x04,
		},

		.interface_control = {
			.bLength = sizeof(usbd_interface_descriptor_t),
			.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE,
			.bInterfaceNumber = SOS_VCP0_INTERFACE,
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
			.acm.bmCapabilities = 0x06, // Device supports the request combination of Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, and the notification Serial_State.
			.union_descriptor.bFunctionLength = sizeof(usbd_cdc_union_functional_descriptor_t),
			.union_descriptor.bDescriptorType = 0x24,
			.union_descriptor.bDescriptorSubType = 0x06, //union descriptor subtype
			.union_descriptor.bMasterInterface = 0x00, //control interface
			.union_descriptor.bSlaveInterface = 0x01, //data interface
			.call_management.bFunctionLength = sizeof(usbd_cdc_call_management_functional_descriptor_t),
			.call_management.bDescriptorType = 0x24,
			.call_management.bDescriptorSubType = 0x01, //call management subtype
			.call_management.bmCapabilities = 0x00, //call management NOT handled -- if call management is handled, the device will be added to Network devices in Mac OS X
			.call_management.bDataInterface = 0x01 //data interface
		},

		.control = {
			.bLength= sizeof(usbd_endpoint_descriptor_t),
			.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT,
			.bEndpointAddress=SOS_LINK_TRANSPORT_USB_INTIN,
			.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_INTERRUPT,
			.wMaxPacketSize=16,
			.bInterval=2
		},

		.interface_data = {
			.bLength = sizeof(usbd_interface_descriptor_t),
			.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE,
			.bInterfaceNumber = SOS_VCP0_DATA_INTERFACE,
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
			.bEndpointAddress=SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_OUT,
			.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK,
			.wMaxPacketSize=SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
			.bInterval=1
		},

		.data_in = {
			.bLength= sizeof(usbd_endpoint_descriptor_t),
			.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT,
			.bEndpointAddress=SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN,
			.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK,
			.wMaxPacketSize=SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
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
const struct sos_link_transport_usb_string_t sos_link_transport_usb_string_desc MCU_WEAK = {
	.bLength = 4,
	.bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING,
	.wLANGID = 0x0409, //English
	.manufacturer = usbd_assign_string(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE, SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING),
	.product = usbd_assign_string(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE, SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING),
	.serial = usbd_assign_string(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE, 0), //dynamically load SN based on silicon
	.vcp0 = usbd_assign_string(SOS_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE, SOS_LINK_TRANSPORT_USB_DESC_VCP_0),
	.vcp1 = usbd_assign_string(SOS_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE, SOS_LINK_TRANSPORT_USB_DESC_VCP_1)
};

int sos_link_usbd_cdc_event_handler(void * object, const mcu_event_t * event){
	usbd_control_t * context = object;

	//if this is a class request check the CDC interfaces
	if ( usbd_control_setup_request_type(context) == USBD_REQUEST_TYPE_CLASS ){
		return cdc_event_handler(context, event);
	}

	return 0;
}


int cdc_event_handler(usbd_control_t * context, const mcu_event_t * event){
	u32 rate = 12000000;
	u32 o_events = event->o_events;
	int iface = usbd_control_setup_interface(context);

	if( (iface == 0) || (iface == 1) || (iface == 2) || (iface == 3) ){

		if ( (o_events & MCU_EVENT_FLAG_SETUP) ){
			switch(context->setup_packet.bRequest){
				case USBD_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND:
				case USBD_CDC_REQUEST_SET_COMM_FEATURE:
				case USBD_CDC_REQUEST_SEND_BREAK:
					//need to receive information from the host
					usbd_control_prepare_buffer(context); //received the incoming data in the buffer
					usbd_control_statusin_stage(context); //data out stage?
					return 1;

				case USBD_CDC_REQUEST_SET_LINE_CODING:
					//usbd_control_statusin_stage(context); //data out stage?
					context->data.dptr = context->buf;
					context->data.nbyte = context->setup_packet.wLength;
					return 1;

				case USBD_CDC_REQUEST_SET_CONTROL_LINE_STATE:
					usbd_control_statusin_stage(context);
					return 1;

				case USBD_CDC_REQUEST_GET_LINE_CODING:
					context->data.dptr = context->buf;

					//copy line coding to dev_std_buf
					context->buf[0] = (rate >>  0) & 0xFF;
					context->buf[1] = (rate >>  8) & 0xFF;
					context->buf[2] = (rate >> 16) & 0xFF;
					context->buf[3] = (rate >> 24) & 0xFF;  //rate
					context->buf[4] =  0; //stop bits 1
					context->buf[5] =  0; //no parity
					context->buf[6] =  8; //8 data bits
					usbd_control_datain_stage(context);
					return 1;

				case USBD_CDC_REQUEST_CLEAR_COMM_FEATURE:
					usbd_control_statusin_stage(context);
					return 1;

				case USBD_CDC_REQUEST_GET_COMM_FEATURE:
					context->data.dptr = context->buf;
					//copy data to dev_std_buf
					usbd_control_statusin_stage(context);
					return 1;

				case USBD_CDC_REQUEST_GET_ENCAPSULATED_RESPONSE:
					context->data.dptr = context->buf;
					//copy data to dev_std_buf
					usbd_control_statusin_stage(context);
					return 1;

				default:
					return 0;
			}

		} else if ( o_events & MCU_EVENT_FLAG_DATA_READY ){
			switch(context->setup_packet.bRequest){
				case USBD_CDC_REQUEST_SET_LINE_CODING:
					//line coding info is available in context->buf
					usbd_control_statusin_stage(context);
					return 1;

				case USBD_CDC_REQUEST_SET_CONTROL_LINE_STATE:
				case USBD_CDC_REQUEST_SET_COMM_FEATURE:
				case USBD_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND:
					//use data in dev_std_buf to take action
					usbd_control_statusin_stage(context);
					return 1;
				default:
					return 0;
			}
		}
	}
	//The request was not handled
	return 0;
}

