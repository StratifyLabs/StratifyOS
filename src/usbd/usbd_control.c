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
#include <unistd.h>
#include <usbd/usbd_standard.h>
#include "sys/ioctl.h"
#include "mcu/mcu.h"
#include "mcu/usb.h"
#include "mcu/boot_debug.h"
#include "mcu/debug.h"
#include "device/sys.h"
#include "usbd/control.h"


static void stall(usbd_control_t * context){
	mcu_usb_stallep(context->handle, (void*)(USBD_ENDPOINT_ADDRESS_IN|0x00));
	context->data.nbyte = 0;
}

void usbd_control_priv_init(void * args){
	usbd_control_t * context = args;
	mcu_action_t action;

	//Set up the action to take when there is data on the control endpoint
	action.channel = 0;
	action.handler.context = context;
	action.handler.callback = usbd_control_handler;
	action.o_events = MCU_EVENT_FLAG_DATA_READY | MCU_EVENT_FLAG_WRITE_COMPLETE;
	action.prio = 0;
	mcu_usb_setaction(context->handle, &action);


	mcu_usb_attach(context->handle, NULL);
}

int usbd_control_handler(void * context_object, mcu_event_t * usb_event /*! Callback data */){
	u32 o_events = usb_event->o_events;
	usbd_control_t * context = context_object;


	if ( o_events & MCU_EVENT_FLAG_SETUP ){
		//read the setup packet
		usbd_control_handler_setup_stage(context);
	}

	if ( o_events & MCU_EVENT_FLAG_SETUP ){
		if (usbd_control_setup_request_type(context) == USBD_REQUEST_STANDARD){
			if( usbd_standard_request_setup_handler(context) == 0 ){
				stall(context);
				return 1;
			}

		} else {

			//a setup event that is not a standard request may be handled by the callback
			if( context->constants->class_event_handler != 0 ){
				if( context->constants->class_event_handler(context_object, usb_event) != 0 ){
					return 1;
				}
			}

			stall(context);
			return 1;
		}

	} else if ( o_events & MCU_EVENT_FLAG_DATA_READY ){ //Data out stage
		if (usbd_control_setup_request_direction(context) == USBD_REQUEST_TYPE_DIRECTION_HOST_TO_DEVICE) {
			if (context->data.nbyte) {
				usbd_control_dataout_stage(context);
				if (context->data.nbyte == 0){

					if (usbd_control_setup_request_type(context) == USBD_REQUEST_STANDARD){
						stall(context);
						return 1;
					}

					//a setup event that is not a standard request may be handled by the callback
					if( context->constants->class_event_handler != 0 ){
						if( context->constants->class_event_handler(context_object, usb_event) != 0 ){
							return 1;
						}
					} else {
						stall(context);
						return 1;
					}
				}
			}
		} else {
			usbd_control_statusout_stage(context);
		}

	} else if ( o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
		if (usbd_control_setup_request_direction(context) == USBD_REQUEST_TYPE_DIRECTION_DEVICE_TO_HOST) {
			usbd_control_datain_stage(context);
		} else {
			if (context->addr & USBD_ENDPOINT_ADDRESS_IN) {
				context->addr &= 0x7F;
				mcu_usb_setaddr(context->handle, (void*)((int)context->addr));
			}
		}
	} else if( o_events & MCU_EVENT_FLAG_STALL ){
		mcu_usb_unstallep(context->handle, (void*)(u32)((usb_event_t*)usb_event->data)->epnum);
	}

	return 1;
}




void * usbd_control_add_ptr(usbd_control_t * context, void * ptr, u32 value){
	return (char*)ptr + value;
}

/*! \details This function reads the setup packet as part of the setup stage.
 */
void usbd_control_handler_setup_stage(usbd_control_t * context){
	mcu_usb_rd_ep(context->handle, 0x00, (uint8_t *)&(context->setup_pkt));
	context->data.nbyte = context->setup_pkt.wLength;
	context->data.max = context->data.nbyte;
}



void usbd_control_datain_stage(usbd_control_t * context) {
	u32 nbyte;
	if (context->data.nbyte > mcu_board_config.usb_max_packet_zero) {
		nbyte = mcu_board_config.usb_max_packet_zero;
	} else {
		nbyte = context->data.nbyte;
	}
	nbyte = mcu_usb_wr_ep(context->handle, 0x80, context->data.dptr, nbyte);
	context->data.dptr += nbyte;
	context->data.nbyte -= nbyte;
}



void usbd_control_dataout_stage(usbd_control_t * context){
	u32 nbyte;
	nbyte = mcu_usb_rd_ep(context->handle, 0x00, context->data.dptr);
	if( context->data.nbyte + MCU_CORE_USB_MAX_PACKET_ZERO_VALUE < context->data.max ){
		context->data.dptr += nbyte;
	}
	context->data.nbyte -= nbyte;
}


//send a zero length packet
void usbd_control_statusin_stage (usbd_control_t * context){
	mcu_usb_wr_ep(context->handle, 0x80, NULL, 0);
}

//receive a zero length packet
void usbd_control_statusout_stage (usbd_control_t * context){
	mcu_usb_rd_ep(context->handle, 0x00, context->buf);
}



