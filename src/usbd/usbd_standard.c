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

#include <usbd/usbd_standard.h>
#include "mcu/usb.h"
#include "mcu/core.h"
#include "mcu/mcu.h"
#include "mcu/boot_debug.h"
#include "mcu/debug.h"
#include "usbd/control.h"

#define USBD_EP_MASK (USBD_ENDPOINT_ADDRESS_IN|(mcu_config.usb_logical_endpoint_count-1))

static int usb_dev_decode_ep(usbd_control_t * context, int ep){
	MCU_UNUSED_ARGUMENT(context);
	if ( ep & USBD_ENDPOINT_ADDRESS_IN ){
		return ((ep << mcu_config.usb_logical_endpoint_count) << (ep & (mcu_config.usb_logical_endpoint_count-1)));
	} else {
		return (1<<ep);
	}
}

static void usbd_control_init_ep(usbd_control_t * context){
	context->ep_halt  = 0;
	context->ep_mask  = 0x00010001;
	context->ep_stall = 0;
}

static void usbd_control_get_serialno(void * dest){
	mcu_sn_t tmp;
	union {
		u8 * b;
		u16 * w;
	} ptr;
	int i, j;
	ptr.w = (u16 *)dest;
	mcu_core_getserialno(&tmp);
	for(j=3; j >= 0; j--){
		for(i=0; i < 8; i++){
			ptr.b[0] = htoc((tmp.sn[j] >> 28) & 0x0F);
			ptr.b[1] = 0;
			tmp.sn[j] <<= 4;
			ptr.w++;
		}
	}
}



char htoc(int nibble){
	if ( nibble >= 0 && nibble < 10 ){
		return (char)nibble + '0';
	} else {
		return (char)nibble + 'A' - 10;
	}
}

int usbd_standard_request_handle_setup(usbd_control_t * context){
	switch(context->setup_packet.bRequest) {

		case USBD_REQUEST_STANDARD_GET_STATUS:
			if ( usbd_standard_request_get_status(context) ) {
				usbd_control_datain_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_CLEAR_FEATURE:
			if ( usbd_standard_request_set_clr_feature(context, 0) ) {
				usbd_control_statusin_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_SET_FEATURE:
			if ( usbd_standard_request_set_clr_feature(context, 1) ) {
				usbd_control_statusin_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_SET_ADDRESS:
			if ( usbd_standard_request_set_address(context) ) {
				usbd_control_statusin_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_GET_DESCRIPTOR:
			if ( usbd_standard_request_get_descriptor(context) ) {
				usbd_control_datain_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_GET_CONFIGURATION:
			if ( usbd_standard_request_get_config(context) ) {
				usbd_control_datain_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_SET_CONFIGURATION:
			if ( usbd_standard_request_set_config(context) ) {
				usbd_control_statusin_stage(context);
				return 1;
			}

			break;

		case USBD_REQUEST_STANDARD_GET_INTERFACE:
			if ( usbd_standard_request_get_interface(context) ) {
				usbd_control_datain_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_SET_INTERFACE:
			if ( usbd_standard_request_set_interface(context) ) {
				usbd_control_statusin_stage(context);
				return 1;
			}
			break;

		case USBD_REQUEST_STANDARD_SET_DESCRIPTOR:
		default:
			break;
	}

	return 0;
}


u32 usbd_standard_request_get_status(usbd_control_t * context) {
	u32 i;
	u32 j;

	u16 * bufp = (u16*)context->buf;
	switch (context->setup_packet.bmRequestType.bitmap_t.recipient){
		case USBD_REQUEST_TYPE_RECIPIENT_DEVICE:
			context->data.dptr = (u8 *)&context->status;
			break;

		case USBD_REQUEST_TYPE_RECIPIENT_INTERFACE:
			if ((context->current_configuration != 0) && (context->setup_packet.wIndex.b[0] < context->num_interfaces)) {
				*bufp = 0;
				context->data.dptr = context->buf;
			} else {
				return 0;
			}
			break;

		case USBD_REQUEST_TYPE_RECIPIENT_ENDPOINT:
			i = context->setup_packet.wIndex.b[0] & USBD_EP_MASK;
			j = usb_dev_decode_ep(context, i);
			if (((context->current_configuration != 0) || ((i & (mcu_config.usb_logical_endpoint_count-1)) == 0)) && (context->ep_mask & j)) {
				*bufp = (context->ep_halt & j) ? 1 : 0;
				context->data.dptr = context->buf;
			} else {
				return 0;
			}
			break;

		default:
			return 0;

	}
	return 1;
}

u32 usbd_standard_request_set_clr_feature(usbd_control_t * context, u32 sc) {
	u32 i;
	u32 j;

	switch (context->setup_packet.bmRequestType.bitmap_t.recipient) {

		case USBD_REQUEST_TYPE_RECIPIENT_DEVICE:
			if (context->setup_packet.wValue.w == USBD_REQUEST_STANDARD_FEATURE_REMOTE_WAKEUP) {
				if (sc) {
					context->status |=  USBD_REQUEST_STANDARD_STATUS_REMOTE_WAKEUP;
				} else {
					context->status &= ~USBD_REQUEST_STANDARD_STATUS_REMOTE_WAKEUP;
				}
			} else {
				return 0;
			}
			break;

		case USBD_REQUEST_TYPE_RECIPIENT_INTERFACE:
			return 0;

		case USBD_REQUEST_TYPE_RECIPIENT_ENDPOINT:
			i = context->setup_packet.wIndex.b[0] & USBD_EP_MASK;
			j = usb_dev_decode_ep(context, i);
			if ((context->current_configuration != 0) && ((i & (mcu_config.usb_logical_endpoint_count-1)) != 0) && (context->ep_mask & j)) {
				if (context->setup_packet.wValue.w == USBD_REQUEST_STANDARD_FEATURE_ENDPOINT_STALL) {
					if (sc) {
						usbd_control_stall_endpoint(context->handle, i);
						context->ep_halt |=  j;
					} else {
						if ((context->ep_stall & j) != 0) {
							return 1;
						}
						usbd_control_unstall_endpoint(context->handle, i);
						context->ep_halt &= ~j;
					}
				} else {
					return 0;
				}
			} else {
				return 0;
			}
			break;

		default:
			return 0;

	}
	return 1;
}

u32 usbd_standard_request_set_address (usbd_control_t * context) {
	usbd_control_set_address(context->handle, context->setup_packet.wValue.b[0]);
	return 1;
}

u32 usbd_standard_request_get_config (usbd_control_t * context) {

	switch (context->setup_packet.bmRequestType.bitmap_t.recipient) {

		case USBD_REQUEST_TYPE_RECIPIENT_DEVICE:
			context->data.dptr = &context->current_configuration;
			break;

		default:
			return 0;
	}
	return 1;
}

u32 usbd_standard_request_set_config (usbd_control_t * context) {
	u32 i;
	u32 j;
	usbd_common_descriptor_t *dptr;
	u8 alt_setting = 0;

	if(context->setup_packet.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TYPE_RECIPIENT_DEVICE){
		if ( context->setup_packet.wValue.b[0] ) {
			dptr = (usbd_common_descriptor_t*)context->constants->config;
			while(dptr->bLength) {

				switch(dptr->bDescriptorType) {

					case USBD_DESCRIPTOR_TYPE_CONFIGURATION:
						if (((usbd_configuration_descriptor_t *)dptr)->bConfigurationValue == context->setup_packet.wValue.b[0]) {
							context->current_configuration = context->setup_packet.wValue.b[0];
							context->num_interfaces = ((usbd_configuration_descriptor_t *)dptr)->bNumInterfaces;
							for (i = 0; i < USBD_ALT_SETTING_SIZE; i++) {
								context->alt_setting[i] = 0;
							}
							for (i = 1; i < mcu_config.usb_logical_endpoint_count; i++) {
								if (context->ep_mask & (1 << i)) {
									usbd_control_disable_endpoint(context->handle, i);
								}
								if (context->ep_mask & ((1 << mcu_config.usb_logical_endpoint_count) << i)) {
									usbd_control_disable_endpoint(context->handle, i|USBD_ENDPOINT_ADDRESS_IN);
								}
							}
							usbd_control_init_ep(context);
							usbd_control_configure(context->handle);

							if (((usbd_configuration_descriptor_t *)dptr)->bmAttributes & USBD_CONFIGURATION_ATTRIBUTES_POWERED_MASK) {
								context->status |=  USBD_REQUEST_STANDARD_STATUS_SELF_POWERED;
							} else {
								context->status &= ~USBD_REQUEST_STANDARD_STATUS_SELF_POWERED;
							}
						} else {
							//jump to the next configuration
							dptr = usbd_control_add_ptr(context, dptr,((usbd_configuration_descriptor_t *)dptr)->wTotalLength);
							continue;
						}
						break;

					case USBD_DESCRIPTOR_TYPE_INTERFACE:
						alt_setting = ((usbd_interface_descriptor_t *)dptr)->bAlternateSetting;
						break;


						//enable all the endpoints in the configuration
					case USBD_DESCRIPTOR_TYPE_ENDPOINT:
						if( alt_setting == 0 ){ //when setting the config, use default alt setting of 0
							i = ((usbd_endpoint_descriptor_t *)dptr)->bEndpointAddress & USBD_EP_MASK;
							j = usb_dev_decode_ep(context, i);
							context->ep_mask |= j;
							usbd_control_configure_endpoint(context->handle, (usbd_endpoint_descriptor_t *)dptr);
							usbd_control_enable_endpoint(context->handle, i);
							usbd_control_reset_endpoint(context->handle, i);
						}
						break;

				}

				dptr = usbd_control_add_ptr(context, dptr, dptr->bLength);
			}
		} else {
			//configuration zero disables all USB configurations -- enter addressed state
			context->current_configuration = 0;
			for (i = 1; i < mcu_config.usb_logical_endpoint_count; i++) {
				if (context->ep_mask & (1 << i)) {
					usbd_control_disable_endpoint(context->handle, i);
				}
				if (context->ep_mask & ((1 << mcu_config.usb_logical_endpoint_count) << i)) {
					usbd_control_disable_endpoint(context->handle, i|USBD_ENDPOINT_ADDRESS_IN);
				}
			}
			usbd_control_init_ep(context);
			usbd_control_unconfigure(context->handle);
		}

		if (context->current_configuration != context->setup_packet.wValue.b[0]) {
			//configuration doesn't exist
			return 0;
		}

		return 1;
	}
	return 0;
}

u32 usbd_standard_request_get_interface(usbd_control_t * context) {

	if( context->setup_packet.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TYPE_RECIPIENT_INTERFACE) {
		if ((context->current_configuration != 0) && (context->setup_packet.wIndex.b[0] < context->num_interfaces)) {
			context->data.dptr = context->alt_setting + context->setup_packet.wIndex.b[0];
			context->data.nbyte = 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
	return 1;
}

u32 usbd_standard_request_set_interface(usbd_control_t * context){
	u32 interface_number = 0;
	u32 alternate_setting = 0;
	u32 prev_interface_number = 0;
	u32 mask = 0;
	u32 i;
	u32 j;
	u32 ret;
	usbd_common_descriptor_t *dptr;

	if (context->setup_packet.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TYPE_RECIPIENT_INTERFACE) {

		if (context->current_configuration == 0){
			//configuration has not been set -- can't operate on the interface
			return 0;
		}

		ret = 0;
		dptr  = (usbd_common_descriptor_t *)context->constants->config;

		while (dptr->bLength) {
			switch (dptr->bDescriptorType) {

				case USBD_DESCRIPTOR_TYPE_CONFIGURATION:
					if (((usbd_configuration_descriptor_t *)dptr)->bConfigurationValue != context->current_configuration) {
						//if this isn't the right configuration, jump to the next configuration
						dptr = usbd_control_add_ptr(context, dptr, ((usbd_configuration_descriptor_t *)dptr)->wTotalLength);
						continue;
					}
					break;

				case USBD_DESCRIPTOR_TYPE_INTERFACE:
					interface_number = ((usbd_interface_descriptor_t *)dptr)->bInterfaceNumber;
					alternate_setting = ((usbd_interface_descriptor_t *)dptr)->bAlternateSetting;
					mask = 0;
					if ((interface_number == context->setup_packet.wIndex.b[0]) && (alternate_setting == context->setup_packet.wValue.b[0])) {
						ret = 1;
						if( interface_number < USBD_ALT_SETTING_SIZE ){
							prev_interface_number = context->alt_setting[interface_number];
							context->alt_setting[interface_number] = (u8)alternate_setting;
						} else {
							return 0;
						}
					}
					break;

				case USBD_DESCRIPTOR_TYPE_ENDPOINT:
					if (interface_number == context->setup_packet.wIndex.b[0]) {
						i = ((usbd_endpoint_descriptor_t *)dptr)->bEndpointAddress & USBD_EP_MASK;
						j = usb_dev_decode_ep(context, i);
						if (alternate_setting == context->setup_packet.wValue.b[0]) {
							context->ep_mask |=  j;
							context->ep_halt &= ~j;
							usbd_control_configure_endpoint(context->handle, (usbd_endpoint_descriptor_t *)dptr);
							usbd_control_enable_endpoint(context->handle, i);
							usbd_control_reset_endpoint(context->handle, i);
							mask |= j;
						} else if ((alternate_setting == prev_interface_number) && ((mask & j) == 0)) {
							context->ep_mask &= ~j;
							context->ep_halt &= ~j;
							usbd_control_disable_endpoint(context->handle, i);
						}
					}
					break;
				default:
					break;
			}
			//move the pointer to the next USB descriptor
			dptr = usbd_control_add_ptr(context, dptr, dptr->bLength);
		}
	} else {
		return 0;
	}

	return ret;
}

u32 usbd_standard_request_get_descriptor(usbd_control_t * context) {
	union {
		u8  * b;
		const usbd_string_descriptor_t * cstr;
		usbd_string_descriptor_t * str;
		const usbd_configuration_descriptor_t * cfg;
	} ptr;

	u32 len = 0;
	u32 i;


	if( context->setup_packet.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TYPE_RECIPIENT_DEVICE) {
		switch (context->setup_packet.wValue.b[1]){

			case USBD_DESCRIPTOR_TYPE_DEVICE:
				//give the device descriptor
				context->data.dptr = (u8 * const)context->constants->device;
				len = sizeof(usbd_device_descriptor_t);
				break;

			case USBD_DESCRIPTOR_TYPE_CONFIGURATION:
				//give the entire configuration
				ptr.cfg = context->constants->config;
				for (i = 0; i != context->setup_packet.wValue.b[0]; i++) {
					if (ptr.cfg->bLength != 0) {
						ptr.b += ptr.cfg->wTotalLength;
					}
				}
				if (ptr.cfg->bLength == 0) {
					return 0;
				}
				context->data.dptr = ptr.b;
				len = ptr.cfg->wTotalLength;
				break;

			case USBD_DESCRIPTOR_TYPE_STRING:
				//give the string
				ptr.cstr = context->constants->string;
				for (i = 0; i != context->setup_packet.wValue.b[0]; i++) {
					if (ptr.cstr->bLength != 0) {
						ptr.b += ptr.cstr->bLength;
					}
				}

				len = ptr.cstr->bLength;
				if ( len == 0){
					return 0;
				}

				//as a special case - generate the device serial number if no string is provided
				if ( ptr.cstr->bString == NULL ){
					//generate the string from the device serial number
					ptr.b = context->buf;
					ptr.str->bLength = 32*2 + 2;
					ptr.str->bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING;
					usbd_control_get_serialno( &(ptr.str->bString) );
					len = ptr.str->bLength;
					context->data.dptr = context->buf;
				} else {
					context->data.dptr = ptr.b;
				}

				break;

			case USBD_DESCRIPTOR_TYPE_QUALIFIER:
				return 0;

			default:
				return 0;
		}

	} else {
		return 0;
	}


	if (context->data.nbyte > len) {
		context->data.nbyte = len;
	}



	return 1;
}

