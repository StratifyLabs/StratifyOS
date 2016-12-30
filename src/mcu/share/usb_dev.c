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


#include <stdio.h>
#include "stratify/usb_dev_cdc.h"
#include "stratify/usb_dev_std.h"
#include <unistd.h>
#include "stratify/usb_dev.h"
#include "sys/ioctl.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "dev/sys.h"

static u32 usb_dev_std_req_setinterface(usb_dev_context_t * context);
static u32 usb_dev_std_req_getinterface(usb_dev_context_t * context);
static void * usb_dev_std_add_ptr(usb_dev_context_t * context, void * ptr, u32 n);
static u32 usb_dev_std_reg_set_clrfeature (usb_dev_context_t * context, u32 sc);
static u32 usb_dev_std_req_getdesc(usb_dev_context_t * context);
static u32 usb_dev_std_req_setaddr (usb_dev_context_t * context);
static u32 usb_dev_std_req_setcfg (usb_dev_context_t * context);
static u32 usb_dev_std_req_getcfg (usb_dev_context_t * context);
static u32 usb_dev_std_req_get_status (usb_dev_context_t * context);
static int usb_dev_decode_ep(usb_dev_context_t * context, int ep);
static void usb_dev_std_init_ep(usb_dev_context_t * context);

void usb_dev_default_event(void * context){}
int usb_dev_default_if_req(void * context, int event){ return 0; }

static void stall(usb_dev_context_t * context){
	mcu_usb_stallep(context->constants->port, (void*)(USB_ENDPOINT_IN|0x00));
	context->ep0_data.cnt = 0;
}

void usb_dev_priv_init(void * args){
	usb_dev_context_t * context = args;
	usb_action_t action;

	//Set up the action to take when there is data on the control endpoint
	action.channel = 0;
	action.context = context;
	action.callback = usb_dev_std_setup;
	action.event = USB_EVENT_DATA_READY;
	action.prio = 0;
	mcu_usb_setaction(context->constants->port, &action);
	action.event = USB_EVENT_WRITE_COMPLETE;
	mcu_usb_setaction(context->constants->port, &action);

	mcu_usb_attach(context->constants->port, NULL);
}

int usb_dev_std_setup(void * context_object, mcu_event_t usb_event /*! Callback data */){
	u32 event = MCU_EVENT_CODE(usb_event);
	usb_dev_context_t * context = context_object;
	if ( event == USB_SETUP_EVENT ){
		usb_dev_std_setup_stage(context);
		context->ep0_data.cnt = context->setup_pkt.wLength;
		if ( context->setup_pkt.bmRequestType.bitmap_t.type == USB_DEV_REQUEST_STANDARD){
			switch (context->setup_pkt.bRequest) {

			case USB_DEV_REQUEST_GET_STATUS:
				if (!usb_dev_std_req_get_status(context)) {
					stall(context); return 1;
				}
				usb_dev_std_datain_stage(context);
				break;

			case USB_DEV_REQUEST_CLEAR_FEATURE:
				if (!usb_dev_std_reg_set_clrfeature(context, 0)) {
					stall(context); return 1;
				}
				usb_dev_std_statusin_stage(context);
				context->constants->feature_event(context);

				break;

			case USB_DEV_REQUEST_SET_FEATURE:
				if (!usb_dev_std_reg_set_clrfeature(context, 1)) {
					stall(context); return 1;
				}
				usb_dev_std_statusin_stage(context);
				context->constants->feature_event(context);
				break;

			case USB_DEV_REQUEST_SET_ADDRESS:
				if (!usb_dev_std_req_setaddr(context)) {
					stall(context); return 1;
				}
				usb_dev_std_statusin_stage(context);
				break;

			case USB_DEV_REQUEST_GET_DESCRIPTOR:
				if (!usb_dev_std_req_getdesc(context)) {
					stall(context); return 1;
				}
				usb_dev_std_datain_stage(context);
				break;

			case USB_DEV_REQUEST_SET_DESCRIPTOR:
				mcu_usb_stallep(context->constants->port, (void*)0x00);
				context->ep0_data.cnt = 0;
				break;

			case USB_DEV_REQUEST_GET_CONFIGURATION:
				if (!usb_dev_std_req_getcfg(context)) {
					stall(context); return 1;
				}
				usb_dev_std_datain_stage(context);
				break;

			case USB_DEV_REQUEST_SET_CONFIGURATION:
				if (!usb_dev_std_req_setcfg(context)) {
					stall(context); return 1;
				}
				usb_dev_std_statusin_stage(context);

				//execute the configure event callback so that the class can handle the change
				context->constants->configure_event(context);

				break;

			case USB_DEV_REQUEST_GET_INTERFACE:
				if (!usb_dev_std_req_getinterface(context)) {
					stall(context); return 1;
				}
				usb_dev_std_datain_stage(context);
				break;

			case USB_DEV_REQUEST_SET_INTERFACE:
				if (!usb_dev_std_req_setinterface(context)) {
					stall(context); return 1;
				}
				usb_dev_std_statusin_stage(context);

				//execute the interface event callback so that the class can handle the change
				context->constants->interface_event(context);

				break;
			default:
				stall(context);
				return 1;
			}

		} else if ( context->setup_pkt.bmRequestType.bitmap_t.type == USB_DEV_REQUEST_CLASS ){

			switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

			case USB_DEV_REQUEST_TO_DEVICE:
				stall(context);
				return 1;

			case USB_DEV_REQUEST_TO_INTERFACE:
				//allow the class to handle the event
				if ( context->constants->adc_if_req(context, event) ) return 1;
				if ( context->constants->msc_if_req(context, event) ) return 1;
				if ( context->constants->cdc_if_req(context, event) ) return 1;
				if ( context->constants->hid_if_req(context, event) ) return 1;
				break;
			}

			stall(context);
			return 1;
		}

	} else if ( event == USB_OUT_EVENT ){
		if (context->setup_pkt.bmRequestType.bitmap_t.dir == USB_DEV_REQUEST_HOST_TO_DEVICE) {

			if (context->ep0_data.cnt) {

				usb_dev_std_dataout_stage(context);
				if (context->ep0_data.cnt == 0){

					switch (context->setup_pkt.bmRequestType.bitmap_t.type) {

					case USB_DEV_REQUEST_STANDARD:
						stall(context); return 1;

					case USB_DEV_REQUEST_CLASS:
						if (1){

							switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

							case USB_DEV_REQUEST_TO_INTERFACE:
								if ( context->constants->adc_if_req(context, event) ) return 1;
								if ( context->constants->msc_if_req(context, event) ) return 1;
								if ( context->constants->cdc_if_req(context, event) ) return 1;
								if ( context->constants->hid_if_req(context, event) ) return 1;

							case USB_DEV_REQUEST_TO_DEVICE:
							case USB_DEV_REQUEST_TO_ENDPOINT:

							default:
								stall(context);
								return 1;
							}
							break;
						}
						break;
					}

				}
			}
		} else {
			usb_dev_std_statusout_stage(context);
		}

	} else if ( event == USB_IN_EVENT){
		if (context->setup_pkt.bmRequestType.bitmap_t.dir == USB_DEV_REQUEST_DEVICE_TO_HOST) {
			usb_dev_std_datain_stage(context);
		} else {
			if (context->addr & USB_ENDPOINT_IN) {
				context->addr &= 0x7F;
				mcu_usb_setaddr(context->constants->port, (void*)((int)context->addr));
			}
		}
	} else if ( event == USB_OUT_EVENT_STALL){
		mcu_usb_unstallep(context->constants->port, (void*)0x00);
	} else if ( event == USB_IN_EVENT_STALL ){
		mcu_usb_unstallep(context->constants->port, (void*)(USB_ENDPOINT_IN|0x00));
	}

	return 1;
}

#define USB_DEV_EP_MASK (USB_ENDPOINT_IN|(USB_LOGIC_EP_NUM-1))

int usb_dev_decode_ep(usb_dev_context_t * context, int ep){
	if ( ep & USB_ENDPOINT_IN ){
		return ((ep << USB_LOGIC_EP_NUM) << (ep & (USB_LOGIC_EP_NUM-1)));
	} else {
		return (1<<ep);
	}
}

void * usb_dev_std_add_ptr(usb_dev_context_t * context, void * ptr, u32 value){
	return (char*)ptr + value;
}


void usb_dev_std_init_ep(usb_dev_context_t * context){
	context->ep_halt  = 0;
	context->ep_mask  = 0x00010001;
	context->ep_stall = 0;
}



u32 usb_dev_std_req_get_status(usb_dev_context_t * context) {
	u32 i;
	u32 j;

	u16 * ep0_bufp = (u16*)context->ep0_buf;
	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient){
	case USB_DEV_REQUEST_TO_DEVICE:
		context->ep0_data.dptr = (u8 *)&context->status;
		break;

	case USB_DEV_REQUEST_TO_INTERFACE:
		if ((context->cfg != 0) && (context->setup_pkt.wIndex.b[0] < context->num_interfaces)) {
			*ep0_bufp = 0;
			context->ep0_data.dptr = context->ep0_buf;
		} else {
			return false;
		}
		break;

	case USB_DEV_REQUEST_TO_ENDPOINT:
		i = context->setup_pkt.wIndex.b[0] & USB_DEV_EP_MASK;
		j = usb_dev_decode_ep(context, i);
		if (((context->cfg != 0) || ((i & (USB_LOGIC_EP_NUM-1)) == 0)) && (context->ep_mask & j)) {
			*ep0_bufp = (context->ep_halt & j) ? 1 : 0;
			context->ep0_data.dptr = context->ep0_buf;
		} else {
			return false;
		}
		break;

	default:
		return false;

	}
	return (true);
}


u32 usb_dev_std_reg_set_clrfeature(usb_dev_context_t * context, u32 sc) {
	u32 i;
	u32 j;

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USB_DEV_REQUEST_TO_DEVICE:
		if (context->setup_pkt.wValue.w == USB_FEATURE_REMOTE_WAKEUP) {
			if (sc) {
				context->status |=  USB_GETSTATUS_REMOTE_WAKEUP;
			} else {
				context->status &= ~USB_GETSTATUS_REMOTE_WAKEUP;
			}
		} else {
			return false;
		}
		break;

	case USB_DEV_REQUEST_TO_INTERFACE:
		return false;

	case USB_DEV_REQUEST_TO_ENDPOINT:
		i = context->setup_pkt.wIndex.b[0] & USB_DEV_EP_MASK;
		j = usb_dev_decode_ep(context, i);
		if ((context->cfg != 0) && ((i & (USB_LOGIC_EP_NUM-1)) != 0) && (context->ep_mask & j)) {
			if (context->setup_pkt.wValue.w == USB_FEATURE_ENDPOINT_STALL) {
				if (sc) {
					mcu_usb_stallep(context->constants->port, (void*)i);
					context->ep_halt |=  j;
				} else {
					if ((context->ep_stall & j) != 0) {
						return (true);
					}
					mcu_usb_unstallep(context->constants->port, (void*)i);
					context->ep_halt &= ~j;
				}
			} else {
				return false;
			}
		} else {
			return false;
		}
		break;

	default:
		return false;

	}
	return (true);
}

extern char htoc(int nibble);

static void usb_dev_std_get_serialno(void * dest){
	sn_t tmp;
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

u32 usb_dev_std_req_getdesc(usb_dev_context_t * context) {
	union {
		u8  * b;
		const usb_string_desc_t * cstr;
		usb_string_desc_t * str;
		const usb_cfg_desc_t * cfg;
	} ptr;
	u32 len;
	u32 i;

	if( context->setup_pkt.bmRequestType.bitmap_t.recipient == USB_DEV_REQUEST_TO_DEVICE) {
		switch (context->setup_pkt.wValue.b[1]) {

		case USB_DEVICE_DESCRIPTOR_TYPE:
			//give the device descriptor
			context->ep0_data.dptr = (u8 * const)context->constants->device;
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
				return false;
			}
			context->ep0_data.dptr = ptr.b;
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
				return false;
			}

			//as a special case - generate the device serial number if no string is provided
			if ( ptr.cstr->bString == NULL ){
				//generate the string from the device serial number
				ptr.b = context->ep0_buf;
				ptr.str->bLength = 32*2 + 2;
				ptr.str->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;
				usb_dev_std_get_serialno( &(ptr.str->bString) );
				len = ptr.str->bLength;
				context->ep0_data.dptr = context->ep0_buf;
			} else {
				context->ep0_data.dptr = ptr.b;
			}

			break;

		default:
			return false;
		}

	} else {
		return false;
	}

	if (context->ep0_data.cnt > len) {
		context->ep0_data.cnt = len;
	}

	return (true);
}

u32 usb_dev_std_req_setaddr (usb_dev_context_t * context) {

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USB_DEV_REQUEST_TO_DEVICE:
		context->addr = USB_ENDPOINT_IN | context->setup_pkt.wValue.b[0];
		break;

	default:
		return false;
	}
	return (true);
}

u32 usb_dev_std_req_getcfg (usb_dev_context_t * context) {

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USB_DEV_REQUEST_TO_DEVICE:
		context->ep0_data.dptr = &context->cfg;
		break;

	default:
		return false;
	}
	return (true);
}

u32 usb_dev_std_req_setcfg (usb_dev_context_t * context) {
	u32 i;
	u32 j;
	usb_common_desc_t *dptr;

	if(context->setup_pkt.bmRequestType.bitmap_t.recipient == USB_DEV_REQUEST_TO_DEVICE){

		if ( context->setup_pkt.wValue.b[0] ) {
			dptr = (usb_common_desc_t*)context->constants->config;
			while (dptr->bLength) {

				switch (dptr->bDescriptorType) {

				case USB_CONFIGURATION_DESCRIPTOR_TYPE:
					if (((usb_cfg_desc_t *)dptr)->bConfigurationValue == context->setup_pkt.wValue.b[0]) {
						context->cfg = context->setup_pkt.wValue.b[0];
						context->num_interfaces = ((usb_cfg_desc_t *)dptr)->bNumInterfaces;
						for (i = 0; i < USB_DEV_ALT_SETTING_SIZE; i++) {
							context->alt_setting[i] = 0;
						}
						for (i = 1; i < USB_LOGIC_EP_NUM; i++) {
							if (context->ep_mask & (1 << i)) {
								mcu_usb_disableep(context->constants->port, (void*)i);
							}
							if (context->ep_mask & ((1 << USB_LOGIC_EP_NUM) << i)) {
								mcu_usb_disableep(context->constants->port, (void*)(i|USB_ENDPOINT_IN));
							}
						}
						usb_dev_std_init_ep(context);
						mcu_usb_configure(context->constants->port, (void*)true);

						if (((usb_cfg_desc_t *)dptr)->bmAttributes & USB_CONFIG_POWERED_MASK) {
							context->status |=  USB_GETSTATUS_SELF_POWERED;
						} else {
							context->status &= ~USB_GETSTATUS_SELF_POWERED;
						}
					} else {
						dptr = usb_dev_std_add_ptr(context, dptr,((usb_cfg_desc_t *)dptr)->wTotalLength);
						continue;
					}
					break;


					//enable all the endpoints in the configuration
				case USB_ENDPOINT_DESCRIPTOR_TYPE:
					i = ((usb_ep_desc_t *)dptr)->bEndpointAddress & USB_DEV_EP_MASK;
					j = usb_dev_decode_ep(context, i);
					context->ep_mask |= j;
					mcu_usb_cfgep(context->constants->port, dptr);
					mcu_usb_enableep(context->constants->port, (void*)i);
					mcu_usb_resetep(context->constants->port, (void*)i);
					break;

				}

				dptr = usb_dev_std_add_ptr(context, dptr, dptr->bLength);
			}
		} else {
			//configuration zero disables all USB configurations
			context->cfg = 0;
			for (i = 1; i < USB_LOGIC_EP_NUM; i++) {
				if (context->ep_mask & (1 << i)) {
					mcu_usb_disableep(context->constants->port, (void*)i);
				}
				if (context->ep_mask & ((1 << USB_LOGIC_EP_NUM) << i)) {
					mcu_usb_disableep(context->constants->port, (void*)(i|USB_ENDPOINT_IN));
				}
			}
			usb_dev_std_init_ep(context);
			mcu_usb_configure(context->constants->port, (void*)false);
		}

		if (context->cfg != context->setup_pkt.wValue.b[0]) {
			return false;
		}

		return true;
	}
	return false;
}

u32 usb_dev_std_req_getinterface(usb_dev_context_t * context) {

	switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

	case USB_DEV_REQUEST_TO_INTERFACE:
		if ((context->cfg != 0) && (context->setup_pkt.wIndex.b[0] < context->num_interfaces)) {
			context->ep0_data.dptr = context->alt_setting + context->setup_pkt.wIndex.b[0];
		} else {
			return false;
		}
		break;

	default:
		return false;

	}
	return (true);
}

u32 usb_dev_std_req_setinterface(usb_dev_context_t * context){
	u32 interface_number = 0;
	u32 alternate_setting = 0;
	u32 prev_interface_number = 0;
	u32 mask = 0;
	u32 i;
	u32 j;
	u32 ret;
	usb_common_desc_t *dptr;

	if (context->setup_pkt.bmRequestType.bitmap_t.recipient == USB_DEV_REQUEST_TO_INTERFACE) {

		if (context->cfg == 0){
			//configuration has not been set -- can't operate on the interface
			return false;
		}

		ret = false;
		dptr  = (usb_common_desc_t *)context->constants->config;

		while (dptr->bLength) {
			switch (dptr->bDescriptorType) {

			case USB_CONFIGURATION_DESCRIPTOR_TYPE:
				if (((usb_cfg_desc_t *)dptr)->bConfigurationValue != context->cfg) {
					//if this isn't the right configuration, jump to the next configuration
					dptr = usb_dev_std_add_ptr(context, dptr, ((usb_cfg_desc_t *)dptr)->wTotalLength);
					continue;
				}
				break;

			case USB_INTERFACE_DESCRIPTOR_TYPE:
				interface_number = ((usb_interface_desc_t *)dptr)->bInterfaceNumber;
				alternate_setting = ((usb_interface_desc_t *)dptr)->bAlternateSetting;
				mask = 0;
				if ((interface_number == context->setup_pkt.wIndex.b[0]) && (alternate_setting == context->setup_pkt.wValue.b[0])) {
					ret = true;
					if( interface_number < USB_DEV_ALT_SETTING_SIZE ){
						prev_interface_number = context->alt_setting[interface_number];
						context->alt_setting[interface_number] = (u8)alternate_setting;
					} else {
						return false;
					}
				}
				break;

			case USB_ENDPOINT_DESCRIPTOR_TYPE:
				if (interface_number == context->setup_pkt.wIndex.b[0]) {
					i = ((usb_ep_desc_t *)dptr)->bEndpointAddress & USB_DEV_EP_MASK;
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
			dptr = usb_dev_std_add_ptr(context, dptr, dptr->bLength);
		}
	} else {
		return false;
	}

	return ret;
}


