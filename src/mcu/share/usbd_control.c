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
#include "sys/ioctl.h"
#include "mcu/mcu.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "mcu/sys.h"
#include "mcu/usbd_control.h"
#include "usbd_standard.h"


static void stall(usbd_control_t * context){
	mcu_usb_stallep(context->constants->port, (void*)(USB_ENDPOINT_IN|0x00));
	context->data.nbyte = 0;
	USBD_DEBUG(0x8000);
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
	mcu_usb_setaction(context->constants->port, &action);


	mcu_usb_attach(context->constants->port, NULL);
}

int usbd_control_handler(void * context_object, mcu_event_t * usb_event /*! Callback data */){
	u32 o_events = usb_event->o_events;
	usbd_control_t * context = context_object;

	USBD_DEBUG(0x1000);


	if ( o_events & MCU_EVENT_FLAG_SETUP ){
		USBD_DEBUG(0x2000);
		usbd_control_handler_setup_stage(context);
	}

	if( context->constants->setup_event != 0 ){
		if( context->constants->setup_event(context_object, usb_event) != 0 ){
			return 1;
		}
	}

	if ( o_events & MCU_EVENT_FLAG_SETUP ){
		if ( context->setup_pkt.bmRequestType.bitmap_t.type == USBD_REQUEST_STANDARD){
			USBD_DEBUG(0x4000);
			switch (context->setup_pkt.bRequest) {

			case USBD_REQUEST_GET_STATUS:
				USBD_DEBUG(0x10000);
				if (!usdd_standard_request_get_status(context)) {
					stall(context); return 1;
				}
				usbd_control_datain_stage(context);
				break;

			case USBD_REQUEST_CLEAR_FEATURE:
				USBD_DEBUG(0x20000);
				if (!usbd_standard_request_set_clr_feature(context, 0)) {
					stall(context); return 1;
				}
				usbd_control_statusin_stage(context);
				context->constants->feature_event(context);
				break;

			case USBD_REQUEST_SET_FEATURE:
				USBD_DEBUG(0x40000);
				if (!usbd_standard_request_set_clr_feature(context, 1)) {
					stall(context); return 1;
				}
				usbd_control_statusin_stage(context);
				context->constants->feature_event(context);
				break;

			case USBD_REQUEST_SET_ADDRESS:
				USBD_DEBUG(0x80000);
				if (!usbd_standard_request_set_address(context)) {
					stall(context); return 1;
				}
				usbd_control_statusin_stage(context);
				break;

			case USBD_REQUEST_GET_DESCRIPTOR:
				USBD_DEBUG(0x100000);
				if (!usbd_standard_request_get_descriptor(context)) {
					stall(context);
					return 1;
				}
				usbd_control_datain_stage(context);
				break;

			case USBD_REQUEST_SET_DESCRIPTOR:
				USBD_DEBUG(0x200000);
				mcu_usb_stallep(context->constants->port, (void*)0x00);
				context->data.nbyte = 0;
				break;

			case USBD_REQUEST_GET_CONFIGURATION:
				USBD_DEBUG(0x400000);
				if (!usbd_standard_request_get_config(context)) {
					stall(context);
					return 1;
				}
				usbd_control_datain_stage(context);
				break;

			case USBD_REQUEST_SET_CONFIGURATION:
				USBD_DEBUG(0x800000);
				if (!usbd_standard_request_set_config(context)) {
					stall(context); return 1;
				}
				usbd_control_statusin_stage(context);

				//execute the configure event callback so that the class can handle the change
				context->constants->configure_event(context);

				break;

			case USBD_REQUEST_GET_INTERFACE:
				USBD_DEBUG(0x1000000);
				if (!usbd_standard_request_get_interface(context)) {
					stall(context); return 1;
				}
				usbd_control_datain_stage(context);
				break;

			case USBD_REQUEST_SET_INTERFACE:
				USBD_DEBUG(0x2000000);
				if (!usbd_standard_request_set_interface(context)) {
					stall(context); return 1;
				}
				usbd_control_statusin_stage(context);

				//execute the interface event callback so that the class can handle the change
				context->constants->interface_event(context);

				break;
			default:
				stall(context);
				return 1;
			}

		} else {
			stall(context);
			return 1;
		}

	} else if ( o_events & MCU_EVENT_FLAG_DATA_READY ){ //Data out stage
		if (context->setup_pkt.bmRequestType.bitmap_t.dir == USBD_REQUEST_HOST_TO_DEVICE) {

			if (context->data.nbyte) {

				usbd_control_dataout_stage(context);
				if (context->data.nbyte == 0){

					switch (context->setup_pkt.bmRequestType.bitmap_t.type) {

					case USBD_REQUEST_STANDARD:
						stall(context);
						return 1;

					case USBD_REQUEST_CLASS:
						if (1){

							switch (context->setup_pkt.bmRequestType.bitmap_t.recipient) {

							case USBD_REQUEST_TO_INTERFACE:
								if ( context->constants->adc_if_req(context, 0) ) return 1;
								if ( context->constants->msc_if_req(context, 0) ) return 1;
								if ( context->constants->cdc_if_req(context, 0) ) return 1;
								if ( context->constants->hid_if_req(context, 0) ) return 1;

							case USBD_REQUEST_TO_DEVICE:
							case USBD_REQUEST_TO_ENDPOINT:

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
			usbd_control_statusout_stage(context);
		}

	} else if ( o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
		USBD_DEBUG(0x4000000);
		if (context->setup_pkt.bmRequestType.bitmap_t.dir == USBD_REQUEST_DEVICE_TO_HOST) {
			usbd_control_datain_stage(context);
		} else {
			USBD_DEBUG(0x8000000);
			if (context->addr & USB_ENDPOINT_IN) {
				context->addr &= 0x7F;
				USBD_DEBUG(0x10000000);
				mcu_usb_setaddr(context->constants->port, (void*)((int)context->addr));
			}
		}
	} else if( o_events & MCU_EVENT_FLAG_STALL ){
		mcu_usb_unstallep(context->constants->port, (void*)(u32)((usb_event_t*)usb_event->data)->epnum);
	}

	return 1;
}




void * usbd_control_add_ptr(usbd_control_t * context, void * ptr, u32 value){
	return (char*)ptr + value;
}

/*! \details This function reads the setup packet as part of the setup stage.
 */
void usbd_control_handler_setup_stage(usbd_control_t * context){
	mcu_usb_rd_ep(context->constants->port, 0x00, (uint8_t *)&(context->setup_pkt));
	context->data.nbyte = context->setup_pkt.wLength;
}


/*! \details
 */

void usbd_control_datain_stage(usbd_control_t * context) {
	u32 nbyte;
	if (context->data.nbyte > mcu_board_config.usb_max_packet_zero) {
		nbyte = mcu_board_config.usb_max_packet_zero;
	} else {
		nbyte = context->data.nbyte;
	}
	nbyte = mcu_usb_wr_ep(context->constants->port, 0x80, context->data.dptr, nbyte);
	context->data.dptr += nbyte;
	context->data.nbyte -= nbyte;
}


/*! \details
 */
void usbd_control_dataout_stage(usbd_control_t * context){
	u32 nbyte;
	nbyte = mcu_usb_rd_ep(context->constants->port, 0x00, context->data.dptr);
	context->data.dptr += nbyte;
	context->data.nbyte -= nbyte;
}


/*! \details
 */
void usbd_control_statusin_stage (usbd_control_t * context){
	mcu_usb_wr_ep(context->constants->port, 0x80, NULL, 0);
}

/*! \details
 */
void usbd_control_statusout_stage (usbd_control_t * context){
	mcu_usb_rd_ep(context->constants->port, 0x00, context->buf);
}



