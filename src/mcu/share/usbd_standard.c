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

#include "mcu/usb.h"
#include "mcu/core.h"
#include "mcu/mcu.h"
#include "usbd_standard.h"

#define USBD_EP_MASK (USB_ENDPOINT_IN|(mcu_config.usb_logical_endpoint_count-1))

static int usb_dev_decode_ep(usbd_control_t * context, int ep){
	if ( ep & USB_ENDPOINT_IN ){
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
	_mcu_core_getserialno(&tmp);
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
		return nibble + '0';
	} else {
		return nibble + 'A' - 10;
	}
}


u32 usdd_standard_request_get_status(usbd_control_t * context) {
	u32 i;
	u32 j;

	u16 * bufp = (u16*)context->buf;
	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient){
	case USBD_REQUEST_TO_DEVICE:
		context->data.dptr = (u8 *)&context->status;
		break;

	case USBD_REQUEST_TO_INTERFACE:
		if ((context->cfg != 0) && (context->setup_pkt.wIndex.b[0] < context->num_interfaces)) {
			*bufp = 0;
			context->data.dptr = context->buf;
		} else {
			return 0;
		}
		break;

	case USBD_REQUEST_TO_ENDPOINT:
		i = context->setup_pkt.wIndex.b[0] & USBD_EP_MASK;
		j = usb_dev_decode_ep(context, i);
		if (((context->cfg != 0) || ((i & (mcu_config.usb_logical_endpoint_count-1)) == 0)) && (context->ep_mask & j)) {
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

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USBD_REQUEST_TO_DEVICE:
		if (context->setup_pkt.wValue.w == USB_FEATURE_REMOTE_WAKEUP) {
			if (sc) {
				context->status |=  USB_GETSTATUS_REMOTE_WAKEUP;
			} else {
				context->status &= ~USB_GETSTATUS_REMOTE_WAKEUP;
			}
		} else {
			return 0;
		}
		break;

	case USBD_REQUEST_TO_INTERFACE:
		return 0;

	case USBD_REQUEST_TO_ENDPOINT:
		i = context->setup_pkt.wIndex.b[0] & USBD_EP_MASK;
		j = usb_dev_decode_ep(context, i);
		if ((context->cfg != 0) && ((i & (mcu_config.usb_logical_endpoint_count-1)) != 0) && (context->ep_mask & j)) {
			if (context->setup_pkt.wValue.w == USB_FEATURE_ENDPOINT_STALL) {
				if (sc) {
					mcu_usb_stallep(context->constants->port, (void*)i);
					context->ep_halt |=  j;
				} else {
					if ((context->ep_stall & j) != 0) {
						return 1;
					}
					mcu_usb_unstallep(context->constants->port, (void*)i);
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

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USBD_REQUEST_TO_DEVICE:
		context->addr = USB_ENDPOINT_IN | context->setup_pkt.wValue.b[0];
		break;

	default:
		return 0;
	}
	return 1;
}

u32 usbd_standard_request_get_config (usbd_control_t * context) {

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USBD_REQUEST_TO_DEVICE:
		context->data.dptr = &context->cfg;
		break;

	default:
		return 0;
	}
	return 1;
}

u32 usbd_standard_request_set_config (usbd_control_t * context) {
	u32 i;
	u32 j;
	usb_common_desc_t *dptr;
	u8 alt_setting = 0;

	if(context->setup_pkt.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TO_DEVICE){

		if ( context->setup_pkt.wValue.b[0] ) {
			dptr = (usb_common_desc_t*)context->constants->config;
			while (dptr->bLength) {

				switch (dptr->bDescriptorType) {

				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (((usb_cfg_desc_t *)dptr)->bConfigurationValue == context->setup_pkt.wValue.b[0]) {
						context->cfg = context->setup_pkt.wValue.b[0];
						context->num_interfaces = ((usb_cfg_desc_t *)dptr)->bNumInterfaces;
						for (i = 0; i < USBD_ALT_SETTING_SIZE; i++) {
							context->alt_setting[i] = 0;
						}
						for (i = 1; i < mcu_config.usb_logical_endpoint_count; i++) {
							if (context->ep_mask & (1 << i)) {
								mcu_usb_disableep(context->constants->port, (void*)i);
							}
							if (context->ep_mask & ((1 << mcu_config.usb_logical_endpoint_count) << i)) {
								mcu_usb_disableep(context->constants->port, (void*)(i|USB_ENDPOINT_IN));
							}
						}
						usbd_control_init_ep(context);
						mcu_usb_configure(context->constants->port, (void*)true);

						if (((usb_cfg_desc_t *)dptr)->bmAttributes & USB_CONFIG_POWERED_MASK) {
							context->status |=  USB_GETSTATUS_SELF_POWERED;
						} else {
							context->status &= ~USB_GETSTATUS_SELF_POWERED;
						}
					} else {
						dptr = usbd_control_add_ptr(context, dptr,((usb_cfg_desc_t *)dptr)->wTotalLength);
						continue;
					}
					break;

				case USB_INTERFACE_DESCRIPTOR_TYPE:
					alt_setting = ((usb_interface_desc_t *)dptr)->bAlternateSetting;
					break;


					//enable all the endpoints in the configuration
				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					if( alt_setting == 0 ){ //when setting the config, use default alt setting of 0
						i = ((usb_ep_desc_t *)dptr)->bEndpointAddress & USBD_EP_MASK;
						j = usb_dev_decode_ep(context, i);
						context->ep_mask |= j;
						mcu_usb_cfgep(context->constants->port, dptr);
						mcu_usb_enableep(context->constants->port, (void*)i);
						mcu_usb_resetep(context->constants->port, (void*)i);
					}
					break;

				}

				dptr = usbd_control_add_ptr(context, dptr, dptr->bLength);
			}
		} else {
			//configuration zero disables all USB configurations
			context->cfg = 0;
			for (i = 1; i < mcu_config.usb_logical_endpoint_count; i++) {
				if (context->ep_mask & (1 << i)) {
					mcu_usb_disableep(context->constants->port, (void*)i);
				}
				if (context->ep_mask & ((1 << mcu_config.usb_logical_endpoint_count) << i)) {
					mcu_usb_disableep(context->constants->port, (void*)(i|USB_ENDPOINT_IN));
				}
			}
			usbd_control_init_ep(context);
			mcu_usb_configure(context->constants->port, (void*)0);
		}

		if (context->cfg != context->setup_pkt.wValue.b[0]) {
			return 0;
		}

		return true;
	}
	return 0;
}

u32 usbd_standard_request_get_interface(usbd_control_t * context) {

	if( context->setup_pkt.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TO_INTERFACE) {
		if ((context->cfg != 0) && (context->setup_pkt.wIndex.b[0] < context->num_interfaces)) {
			context->data.dptr = context->alt_setting + context->setup_pkt.wIndex.b[0];
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
	usb_common_desc_t *dptr;

	if (context->setup_pkt.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TO_INTERFACE) {

		if (context->cfg == 0){
			//configuration has not been set -- can't operate on the interface
			return 0;
		}

		ret = 0;
		dptr  = (usb_common_desc_t *)context->constants->config;

		while (dptr->bLength) {
			switch (dptr->bDescriptorType) {

			case USB_CONFIGURATION_DESCRIPTOR_TYPE:
				if (((usb_cfg_desc_t *)dptr)->bConfigurationValue != context->cfg) {
					//if this isn't the right configuration, jump to the next configuration
					dptr = usbd_control_add_ptr(context, dptr, ((usb_cfg_desc_t *)dptr)->wTotalLength);
					continue;
				}
				break;

			case USB_INTERFACE_DESCRIPTOR_TYPE:
				interface_number = ((usb_interface_desc_t *)dptr)->bInterfaceNumber;
				alternate_setting = ((usb_interface_desc_t *)dptr)->bAlternateSetting;
				mask = 0;
				if ((interface_number == context->setup_pkt.wIndex.b[0]) && (alternate_setting == context->setup_pkt.wValue.b[0])) {
					ret = true;
					if( interface_number < USBD_ALT_SETTING_SIZE ){
						prev_interface_number = context->alt_setting[interface_number];
						context->alt_setting[interface_number] = (u8)alternate_setting;
					} else {
						return 0;
					}
				}
				break;

			case USB_ENDPOINT_DESCRIPTOR_TYPE:
				if (interface_number == context->setup_pkt.wIndex.b[0]) {
					i = ((usb_ep_desc_t *)dptr)->bEndpointAddress & USBD_EP_MASK;
					j = usb_dev_decode_ep(context, i);
					if (alternate_setting == context->setup_pkt.wValue.b[0]) {
						context->ep_mask |=  j;
						context->ep_halt &= ~j;
						mcu_usb_cfgep(context->constants->port, dptr);
						mcu_usb_enableep(context->constants->port, (void*)i);
						mcu_usb_resetep(context->constants->port, (void*)i);
						mask |= j;
					} else if ((alternate_setting == prev_interface_number) && ((mask & j) == 0)) {
						context->ep_mask &= ~j;
						context->ep_halt &= ~j;
						mcu_usb_disableep(context->constants->port, (void*)i);
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
		const usb_string_desc_t * cstr;
		usb_string_desc_t * str;
		const usb_cfg_desc_t * cfg;
	} ptr;
	u32 len;
	u32 i;

	if( context->setup_pkt.bmRequestType.bitmap_t.recipient == USBD_REQUEST_TO_DEVICE) {
		switch (context->setup_pkt.wValue.b[1]) {

		case USB_DEVICE_DESCRIPTOR_TYPE:
			//give the device descriptor
			context->data.dptr = (u8 * const)context->constants->device;
			len = sizeof(usb_dev_desc_t);
			break;

		case USB_CONFIGURATION_DESCRIPTOR_TYPE:
			//give the entire configuration
			ptr.cfg = context->constants->config;
			for (i = 0; i != context->setup_pkt.wValue.b[0]; i++) {
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

		case USB_STRING_DESCRIPTOR_TYPE:
			//give the string
			ptr.cstr = context->constants->string;
			for (i = 0; i != context->setup_pkt.wValue.b[0]; i++) {
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
				ptr.str->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;
				usbd_control_get_serialno( &(ptr.str->bString) );
				len = ptr.str->bLength;
				context->data.dptr = context->buf;
			} else {
				context->data.dptr = ptr.b;
			}

			break;

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

