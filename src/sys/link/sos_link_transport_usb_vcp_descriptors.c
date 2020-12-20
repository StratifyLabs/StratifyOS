// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "config.h"
#include "sos/link.h"
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "usbd/control.h"
#include "usbd/cdc.h"
#include "sos/debug.h"
#include "device/sys.h"

#include "sos/link/transport_usb_vcp.h"


#define SOS_REQUIRED_CURRENT 500

#define VCP0_INTERFACE 0
#define VCP0_INTERFACE_STRING 4

static int cdc_event_handler(usbd_control_t * context, const mcu_event_t * event);

SOS_LINK_TRANSPORT_USB_CONST(
  vcp,
  usbd_cdc_event_handler)

SOS_LINK_TRANSPORT_USB_DEVICE_DESCRIPTOR(vcp,USBD_DEVICE_CLASS_COMMUNICATIONS,0,0,SOS_LINK_TRANSPORT_USB_BCD_VERSION | 1)

const sos_link_transport_usb_vcp_configuration_descriptor_t sos_link_transport_usb_vcp_configuration_descriptor MCU_WEAK = {

	.cfg = {
		.bLength = sizeof(usbd_configuration_descriptor_t),
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_CONFIGURATION,
		.wTotalLength = sizeof(sos_link_transport_usb_vcp_configuration_descriptor_t)-1, //exclude the zero terminator
		.bNumInterfaces = 0x02,
		.bConfigurationValue = 0x01,
		.iConfiguration = 2,
		.bmAttributes = USBD_CONFIGURATION_ATTRIBUTES_BUS_POWERED,
		.bMaxPower = USBD_CONFIGURATION_MAX_POWER_MA( SOS_REQUIRED_CURRENT )
	},

	.vcp = {
		USBD_CDC_DECLARE_CONFIGURATION_DESCRIPTOR(
		VCP0_INTERFACE_STRING,
		VCP0_INTERFACE_STRING,
		VCP0_INTERFACE,
		VCP0_INTERFACE+1,
		SOS_LINK_TRANSPORT_USB_VCP_INTERRUPT_ENDPOINT,
		SOS_LINK_TRANSPORT_USB_VCP_INTERRUPT_ENDPOINT_SIZE,
		SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT,
		SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE
		)
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
const struct sos_link_transport_usb_vcp_string_descriptor_t sos_link_transport_usb_vcp_string_descriptor MCU_WEAK = {
	.bLength = 4,
	.bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING,
	.wLANGID = 0x0409, //English
	.manufacturer = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE, SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING),
	.product = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE, SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING),
	.serial = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE, 0), //dynamically load SN based on silicon
	.vcp = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_VCP_SIZE, SOS_LINK_TRANSPORT_USB_DESC_VCP),
};

int usbd_cdc_event_handler(void * object, const mcu_event_t * event){
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

	if( (iface == VCP0_INTERFACE) || (iface == VCP0_INTERFACE+1) ){

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

