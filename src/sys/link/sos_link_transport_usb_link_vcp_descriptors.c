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

#include "sos/link/transport_usb_link_vcp.h"
#include "sos/link/transport_usb_vcp.h"


#define SOS_REQUIRED_CURRENT 500

#define INTERFACE_NUMBER 0
#define INTERFACE_STRING 4

#define VCP0_INTERFACE 1
#define VCP0_INTERFACE_STRING 5

typedef struct MCU_PACK {
	usbd_msft_compatible_id_header_feature_descriptor_t header;
	usbd_msft_compatible_id_interface_feature_descriptor_t interface_feature0;
	usbd_msft_compatible_id_interface_feature_descriptor_t interface_feature1;
	//usbd_msft_compatible_id_interface_feature_descriptor_t interface_feature2;
} msft_os1_compatible_id_feature_descriptor_t;

static const msft_os1_compatible_id_feature_descriptor_t msft_os1_compatible_id_feature_descriptor =
{
	.header = {
		.length = sizeof(msft_os1_compatible_id_feature_descriptor_t),
		.bcd = 0x0100,
		.compatible_id_index = 0x0004,
		.section_count[0] = 1,
	},
	.interface_feature0 = {
		.interface_number = 0,
		.resd0 = 0x01,
		.compatible_id = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, //WINUSB\0\0
	}
	,
	.interface_feature1 = {
		.interface_number = 1,
		.resd0 = 0x01
	}
		#if 0
	,
	.interface_feature2 = {
		.interface_number = 2,
		.resd0 = 0x01
	}
		#endif
};

typedef struct MCU_PACK {
	usbd_msft_os2_function_subset_header_t header_descriptor;
	usbd_msft_os2_compatible_id_t compatible_id_descriptor;
} interface_descriptor_t;


typedef struct MCU_PACK {
	usbd_msft_os2_configuration_subset_header_t header_descriptor;
	interface_descriptor_t interface_descriptor;
} configuration_descriptor_t;


typedef struct MCU_PACK {
	usbd_msft_os2_descriptor_set_header_t header_descriptor;
	configuration_descriptor_t configuration_descriptor;
} compatible_id_feature_descriptor_t;

#if 0
static const compatible_id_feature_descriptor_t msft_compatible_id_feature_descriptor =
{
	.header_descriptor = {
		.wLength = sizeof(usbd_msft_os2_descriptor_set_header_t),
		.wDescriptorType = USBD_MSFT_OS2_SET_HEADER_DESCRIPTOR,
		.dwWindowsVersion = 0x06030000, //windows 8.1
		.wTotalLength= sizeof(compatible_id_feature_descriptor_t),
	},
	.configuration_descriptor = {
		.header_descriptor = {
			.wLength = sizeof(usbd_msft_os2_configuration_subset_header_t),
			.wDescriptorType = USBD_MSFT_OS2_SUBSET_HEADER_CONFIGURATION,
			.bConfigurationIndex = 0,
			.wTotalLength = sizeof(configuration_descriptor_t)
		},
		.interface_descriptor = {
			.header_descriptor = {
				.wLength = sizeof(usbd_msft_os2_function_subset_header_t),
				.wDescriptorType = USBD_MSFT_OS2_SUBSET_HEADER_CONFIGURATION,
				.bFirstInterface = 0,
				.wSubsetLength = sizeof(interface_descriptor_t)
			},
			.compatible_id_descriptor = {
				.wLength = sizeof(usbd_msft_os2_compatible_id_t),
				.wDescriptorType = USBD_MSFT_OS2_FEATURE_COMPATIBLE_ID,
				.CompatibleID = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, //WINUSB\0\0
			}
		}
	}
};
#endif

const usbd_msft_bos_descriptor_t bos_descriptor = {
	.bos_descriptor = {
		.bLength = sizeof(usbd_bos_descriptor_t),
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_BOS,
		.wTotalLength = sizeof(usbd_msft_bos_descriptor_t),
		.bNumDeviceCaps = 1
	},
	.bLength = sizeof(usbd_msft_bos_descriptor_t) - sizeof(usbd_bos_descriptor_t),
	.bDescriptorType = 0x10,
	.bDevCapabilityType = 0x05,
	.bReserved = 0,
	.PlatformCapabilityUUID = {
		0xDF, 0x60, 0xDD, 0xD8,
		0x89, 0x45, 0xC7, 0x4C,
		0x9C, 0xD2, 0x65, 0x9D,
		0x9E, 0x64, 0x8A, 0x9F
	},
	.dwWindowsVersion = 0x06030000,
	.wMSOSDescriptorSetTotalLength = sizeof(compatible_id_feature_descriptor_t),
	.bMS_VendorCode = SOS_LINK_TRANSPORT_MSFT_VENDOR_CODE,
	.bAltEnumCode = 0
};

int link_vcp_class_handler(void * object, const mcu_event_t * event){
	usbd_control_t * context = object;
	u32 o_events = event->o_events;

	if( sos_link_transport_usb_msft_string_event(object,event) ){
		mcu_debug_printf("send MSFT100\n");
		return 1;
	}

	if( (o_events & MCU_EVENT_FLAG_SETUP)
			&& (context->setup_packet.bRequest == USBD_MSFT_VENDOR_CODE_BYTE)
			&& (context->setup_packet.wIndex.w == 0x0004)){


		mcu_debug_printf("request 0x%X 0x%X 0x%X\n",
										 context->setup_packet.bmRequestType,
										 context->setup_packet.bRequest,
										 context->setup_packet.wValue.w);

		u16 len = sizeof(msft_os1_compatible_id_feature_descriptor);
		context->data.dptr = (u8*)&msft_os1_compatible_id_feature_descriptor;
		if (context->data.nbyte > len) {
			context->data.nbyte = len;
		}
		mcu_debug_printf("send OS descriptors %d\n", len);
		usbd_control_datain_stage(context);
		return 1;


#if 0
		if( context->setup_packet.bRequest == SOS_LINK_TRANSPORT_MSFT_VENDOR_CODE ){
			//respond to SOS_LINK_TRANSPORT_MSFT_VENDOR_CODE request
			context->data.dptr = (u8 * const)&msft_compatible_id_feature_descriptor;
			if (context->data.nbyte > sizeof(msft_compatible_id_feature_descriptor)) {
				context->data.nbyte = sizeof(msft_compatible_id_feature_descriptor);
			}
			usbd_control_datain_stage(context);
			return 1;

		} else if(
							(context->setup_packet.bRequest == USBD_REQUEST_STANDARD_GET_DESCRIPTOR)
							&& (context->setup_packet.wValue.b[1] == USBD_DESCRIPTOR_TYPE_BOS )
							){
			//respond to GET DESCRIPTOR for BOS
			context->data.dptr = (u8 * const)&bos_descriptor;
			if (context->data.nbyte > sizeof(bos_descriptor)) {
				context->data.nbyte = sizeof(bos_descriptor);
			}
			usbd_control_datain_stage(context);

			//tell caller this is handled
			return 1;
		}
#endif
	}

	return sos_link_usbd_cdc_event_handler(object, event);
}

SOS_LINK_TRANSPORT_USB_DEVICE_DESCRIPTOR(link_vcp,239,2,1,SOS_LINK_TRANSPORT_USB_BCD_VERSION | 2)

SOS_LINK_TRANSPORT_USB_CONST(
		link_vcp,
		SOS_LINK_TRANSPORT_USB_PORT,
		0,
		0,
		link_vcp_class_handler
		)


const sos_link_transport_usb_link_vcp_configuration_descriptor_t
sos_link_transport_usb_link_vcp_configuration_descriptor MCU_WEAK = {

		.cfg = {
		.bLength = sizeof(usbd_configuration_descriptor_t),
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_CONFIGURATION,
		.wTotalLength = sizeof(sos_link_transport_usb_link_vcp_configuration_descriptor_t)-1, //exclude the zero terminator
		.bNumInterfaces = 3,
		.bConfigurationValue = 0x01,
		.iConfiguration = 2,
		.bmAttributes = USBD_CONFIGURATION_ATTRIBUTES_BUS_POWERED,
		.bMaxPower = USBD_CONFIGURATION_MAX_POWER_MA( SOS_REQUIRED_CURRENT )
		},

		.link_interface = {
		SOS_LINK_TRANSPORT_USB_DECLARE_INTERFACE_CONFIGURATION_DESCRIPTOR(
			INTERFACE_STRING,
			INTERFACE_NUMBER,
			SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT,
			SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE
			)
		},

		.vcp = {
		USBD_CDC_DECLARE_CONFIGURATION_DESCRIPTOR(
			VCP0_INTERFACE_STRING,
			VCP0_INTERFACE_STRING,
			VCP0_INTERFACE,
			VCP0_INTERFACE+1,
			SOS_LINK_TRANSPORT_USB_LINK_VCP_INTERRUPT_ENDPOINT,
			SOS_LINK_TRANSPORT_USB_LINK_VCP_INTERRUPT_ENDPOINT_SIZE,
			SOS_LINK_TRANSPORT_USB_LINK_VCP_BULK_ENDPOINT,
			SOS_LINK_TRANSPORT_USB_LINK_VCP_BULK_ENDPOINT_SIZE
			)
		},

		.terminator = 0
		};

const struct sos_link_transport_usb_link_vcp_string_descriptor_t sos_link_transport_usb_link_vcp_string_descriptor MCU_WEAK = {
	.bLength = 4,
	.bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING,
	.wLANGID = 0x0409, //English
	.manufacturer = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE, SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING),
	.product = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE, SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING),
	.serial = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE, 0), //dynamically load SN based on silicon
	.link = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK_SIZE, SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK),
	.vcp = USBD_ASSIGN_STRING(SOS_LINK_TRANSPORT_LINK_USB_VCP_DESCRIPTOR_SIZE, SOS_LINK_TRANSPORT_LINK_USB_VCP_DESCRIPTOR)
};

