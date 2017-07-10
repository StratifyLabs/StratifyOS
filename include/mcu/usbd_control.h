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

#ifndef USBD_H_
#define USBD_H_

#include "sos/dev/usb.h"
#include "mcu/types.h"

#include "usbd_defs.h"
#include "usbd_typedefs.h"

//#define USBD_PORT 0
#define USBD_ALT_SETTING_SIZE 16



typedef struct {
	const u8 port;
	const void * const device;
	const void * const config;
	const void * const string;
	int (* const setup_event)(void * context, mcu_event_t * event);
	void (*const feature_event)(void * context);
	void (*const configure_event)(void * context);
	void (*const interface_event)(void * context);
	int (*adc_if_req)(void * context, int event);
	int (*msc_if_req)(void * context, int event);
	int (*cdc_if_req)(void * context, int event);
	int (*hid_if_req)(void * context, int event);
} usbd_control_constants_t;

#define MCU_CORE_USB_MAX_PACKET_ZERO_VALUE 64

typedef struct {
  u8 * dptr;
  u16 cnt;
} usbd_control_data_t;

typedef struct MCU_PACK {
	usb_setup_pkt_t setup_pkt;
	usbd_control_data_t ep0_data;
	u16 status;
	u8 addr;
	u8 cfg;
	u32 ep_mask;
	u32 ep_halt;
	u32 ep_stall;
	u8 num_interfaces;
	u8 ep0_buf[MCU_CORE_USB_MAX_PACKET_ZERO_VALUE];
	u8 alt_setting[USBD_ALT_SETTING_SIZE];
	const usbd_control_constants_t * constants;
} usbd_control_t;

int usbd_control_handler(void * context, mcu_event_t * data);
void usbd_control_priv_init(void * context);


void usbd_control_handler_stage(usbd_control_t * context);
void usbd_control_datain_stage(usbd_control_t * context) MCU_NEVER_INLINE;
void usbd_control_dataout_stage (usbd_control_t * context);
void usbd_control_statusin_stage(usbd_control_t * context);
void usbd_control_statusout_stage (usbd_control_t * context);


char htoc(int nibble);



#endif /* USB_DEVICE_H_ */

/*! @} */
