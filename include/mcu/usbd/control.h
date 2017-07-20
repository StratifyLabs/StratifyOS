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

/*! \addtogroup USB_DEV USB Device
 *
 *
 * @{
 * \ingroup STFY
 *
 */

#ifndef MCU_USBD_CONTROL_H_
#define MCU_USBD_CONTROL_H_

#include "sos/dev/usb.h"
#include "mcu/types.h"

#include "defines.h"
#include "typedefs.h"

//#define USBD_PORT 0
#define USBD_ALT_SETTING_SIZE 16



typedef struct {
	const u8 port;
	const void * const device;
	const void * const config;
	const void * const string;
	int (* const class_event_handler)(void * context, mcu_event_t * event);
} usbd_control_constants_t;

#define MCU_CORE_USB_MAX_PACKET_ZERO_VALUE 64

#define USBD_CONTROL_DATAOUT_BUF_SIZE 256

typedef struct {
  u8 * dptr;
  u16 nbyte;
} usbd_control_data_t;

typedef struct MCU_PACK {
	usbd_setup_packet_t setup_pkt;
	usbd_control_data_t data;
	u16 status;
	u8 addr;
	u8 cfg;
	u32 ep_mask;
	u32 ep_halt;
	u32 ep_stall;
	u8 num_interfaces;
	u8 buf[MCU_CORE_USB_MAX_PACKET_ZERO_VALUE];
	u8 alt_setting[USBD_ALT_SETTING_SIZE];
	const usbd_control_constants_t * constants;
} usbd_control_t;

void usbd_control_priv_init(void * context);
int usbd_control_handler(void * context, mcu_event_t * data);

void usbd_control_handler_setup_stage(usbd_control_t * context);
void usbd_control_datain_stage(usbd_control_t * context) MCU_NEVER_INLINE;

void usbd_control_dataout_stage (usbd_control_t * context);
void usbd_control_statusin_stage(usbd_control_t * context);
void usbd_control_statusout_stage (usbd_control_t * context);

static inline int usbd_control_setup_request_type(usbd_control_t * context){
	return context->setup_pkt.bmRequestType.bitmap_t.type;
}

static inline int usbd_control_setup_interface(usbd_control_t * context){
	return context->setup_pkt.wIndex.b[0];
}

static inline int usbd_control_setup_request(usbd_control_t * context){
	return context->setup_pkt.bRequest;
}

static inline int usbd_control_setup_request_direction(usbd_control_t * context){
	return context->setup_pkt.bmRequestType.bitmap_t.dir;
}


char htoc(int nibble);



#endif /* MCU_USBD_CONTROL_H_ */

/*! @} */
