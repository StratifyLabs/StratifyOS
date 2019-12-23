/* Copyright 2011-2017 Tyler Gilbert;
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
#include <unistd.h>
#include <usbd/usbd_standard.h>
#include "sys/ioctl.h"
#include "mcu/mcu.h"
#include "mcu/usb.h"
#include "mcu/boot_debug.h"
#include "mcu/debug.h"
#include "device/sys.h"
#include "usbd/control.h"


static int execute_class_handler(usbd_control_t * context, const mcu_event_t * usb_event){
	if( context->constants->class_event_handler != 0 ){
		return context->constants->class_event_handler(context, usb_event);
	}
	return 0;
}

static void stall(usbd_control_t * context){
	usbd_control_stall_endpoint(context->handle, (USBD_ENDPOINT_ADDRESS_IN|0x00));
	context->data.nbyte = 0;
}

void usbd_control_svcall_init(void * args){
	CORTEXM_SVCALL_ENTER();
	usbd_control_root_init(args);
}

void usbd_control_root_init(void * args){
	usbd_control_t * context = args;
	mcu_action_t action;

	//Set up the action to take when there is data on the control endpoint
	action.channel = 0;
	action.handler.context = context;
	action.handler.callback = usbd_control_handler;
	action.o_events = MCU_EVENT_FLAG_DATA_READY | MCU_EVENT_FLAG_WRITE_COMPLETE | MCU_EVENT_FLAG_SETUP;
	action.prio = 0;
	if( mcu_usb_setaction(context->handle, &action) < 0 ){
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "usbd control setaction");
	}

	if( usbd_control_attach(context->handle) < 0 ){
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, "usbd control setaction");
	}
}

int usbd_control_handler(void * context_object, const mcu_event_t * usb_event /*! Callback data */){
	u32 o_events = usb_event->o_events;
	usbd_control_t * context = context_object;
	int ret;

	if ( o_events & MCU_EVENT_FLAG_SETUP ){
		//read the setup packet
		usbd_control_handler_setup_stage(context);
	}

	if ( o_events & MCU_EVENT_FLAG_SETUP ){
		ret = usbd_standard_request_handle_setup(context);

		//allow the class handler handle the standard request if the request was handled with usbd_standard_request_handle_setup(), no need to stall
		if( (execute_class_handler(context, usb_event) == 0) &&
				(ret == 0) ){
			stall(context);
		}

	} else if ( o_events & MCU_EVENT_FLAG_DATA_READY ){ //Data out stage
		if (
			 usbd_control_setup_request_direction(context) ==
			 USBD_REQUEST_TYPE_DIRECTION_HOST_TO_DEVICE
			 ) {
			if( context->data.nbyte ){
				usbd_control_dataout_stage(context);
				if (context->data.nbyte == 0){
					if (
						 usbd_control_setup_request_type(context) ==
						 USBD_REQUEST_STANDARD
						 ){
						stall(context);
					} else if( execute_class_handler(context, usb_event) == 0 ){
						stall(context);
					}
				}
			}
		} else {
			usbd_control_statusout_stage(context);
		}

	} else if ( o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
		if (usbd_control_setup_request_direction(context) == USBD_REQUEST_TYPE_DIRECTION_DEVICE_TO_HOST) {

			if( execute_class_handler(context, usb_event) ){
				return 1;
			}

			usbd_control_datain_stage(context);
		}
	} else if( o_events & MCU_EVENT_FLAG_STALL ){
		usbd_control_unstall_endpoint(context->handle, ((usb_event_t*)usb_event->data)->epnum);
	}

	return 1;
}

void * usbd_control_add_ptr(usbd_control_t * context, void * ptr, u32 value){
	return (char*)ptr + value;
}

void usbd_control_handler_setup_stage(usbd_control_t * context){
	mcu_usb_root_read_endpoint(context->handle, 0x00, (u8 *)&(context->setup_packet));
	context->data.nbyte = context->setup_packet.wLength;
	context->data.is_zlp = 0;
}

void usbd_control_datain_stage(usbd_control_t * context) {
	u32 nbyte;

	//we can only send max packet size at a time
	if (context->data.nbyte >= mcu_board_config.usb_max_packet_zero) {
		nbyte = mcu_board_config.usb_max_packet_zero;
		context->data.is_zlp = 1;
	} else {
		nbyte = context->data.nbyte;
		context->data.is_zlp = 0;
	}

	if( (nbyte > 0) ||
		 (context->data.is_zlp) //need to send a ZLP to mark the end
		 ){
		mcu_usb_root_write_endpoint(context->handle, 0x80, context->data.dptr, nbyte);
		context->data.dptr += nbyte;
		context->data.nbyte -= nbyte;
		if( nbyte == 0 ){
			context->data.is_zlp = 0;
		}
	}


}

void usbd_control_dataout_stage(usbd_control_t * context){
	u32 nbyte;
	nbyte = mcu_usb_root_read_endpoint(context->handle, 0x00, context->data.dptr);
	if( nbyte > context->data.nbyte ){
		nbyte = context->data.nbyte;
	}
	context->data.dptr += nbyte;
	context->data.nbyte -= nbyte;
}


//send a zero length packet
void usbd_control_statusin_stage(usbd_control_t * context){
	mcu_usb_root_write_endpoint(context->handle, 0x80, NULL, 0);
}

//receive a zero length packet
void usbd_control_statusout_stage(usbd_control_t * context){
	mcu_usb_root_read_endpoint(context->handle, 0x00, context->buf);
}

int usbd_control_reset(const devfs_handle_t * handle){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_RESET;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_attach(const devfs_handle_t * handle){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_ATTACH;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_detach(const devfs_handle_t * handle){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_DETACH;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_configure(const devfs_handle_t * handle){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_CONFIGURE;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_unconfigure(const devfs_handle_t * handle){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_UNCONFIGURE;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_set_address(const devfs_handle_t * handle, u8 address){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_SET_ADDRESS;
	attr.address = address;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_reset_endpoint(const devfs_handle_t * handle, u8 endpoint_number){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_RESET_ENDPOINT;
	attr.address = endpoint_number;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_enable_endpoint(const devfs_handle_t * handle, u8 endpoint_number){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_ENABLE_ENDPOINT;
	attr.address = endpoint_number;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_disable_endpoint(const devfs_handle_t * handle, u8 endpoint_number){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_DISABLE_ENDPOINT;
	attr.address = endpoint_number;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_stall_endpoint(const devfs_handle_t * handle, u8 endpoint_number){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_STALL_ENDPOINT;
	attr.address = endpoint_number;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_unstall_endpoint(const devfs_handle_t * handle, u8 endpoint_number){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_UNSTALL_ENDPOINT;
	attr.address = endpoint_number;
	return mcu_usb_setattr(handle, &attr);
}

int usbd_control_configure_endpoint(const devfs_handle_t * handle, const usbd_endpoint_descriptor_t * endpoint_descriptor){
	usb_attr_t attr;
	attr.o_flags = USB_FLAG_CONFIGURE_ENDPOINT;
	attr.address = endpoint_descriptor->bEndpointAddress;
	attr.max_packet_size = endpoint_descriptor->wMaxPacketSize;
	attr.type = endpoint_descriptor->bmAttributes;
	return mcu_usb_setattr(handle, &attr);
}




