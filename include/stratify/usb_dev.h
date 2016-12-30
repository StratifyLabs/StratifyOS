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

#ifndef USB_DEV_H_
#define USB_DEV_H_

#include "mcu/mcu.h"
#include "mcu/pio.h"
#include "iface/dev/usb.h"
#include "../stratify/usb_dev_defs.h"
#include "../stratify/usb_dev_typedefs.h"

//#define USB_DEV_PORT 0
#define USB_DEV_ALT_SETTING_SIZE 16

typedef struct {
  u8 * dptr;
  u16 cnt;
} usb_ep_data_t;

typedef struct {
	const u8 port;
	const void * const device;
	const void * const config;
	const void * const string;
	void (*const feature_event)(void * context);
	void (*const configure_event)(void * context);
	void (*const interface_event)(void * context);
	int (*adc_if_req)(void * context, int event);
	int (*msc_if_req)(void * context, int event);
	int (*cdc_if_req)(void * context, int event);
	int (*hid_if_req)(void * context, int event);
} usb_dev_const_t;


typedef struct MCU_PACK {
	usb_setup_pkt_t setup_pkt;
	usb_ep_data_t ep0_data;
	u16 status;
	u8 addr;
	u8 cfg;
	u32 ep_mask;
	u32 ep_halt;
	u32 ep_stall;
	u8 num_interfaces;
	u8 ep0_buf[MCU_CORE_USB_MAX_PACKET_ZERO_VALUE];
	u8 alt_setting[USB_DEV_ALT_SETTING_SIZE];
	const usb_dev_const_t * constants;
} usb_dev_context_t;

int usb_dev_std_setup(void * context, mcu_event_t data);
void usb_dev_std_setup_stage(usb_dev_context_t * context);
void usb_dev_std_datain_stage(usb_dev_context_t * context) MCU_NEVER_INLINE;
void usb_dev_std_dataout_stage (usb_dev_context_t * context);
void usb_dev_std_statusin_stage(usb_dev_context_t * context);
void usb_dev_std_statusout_stage (usb_dev_context_t * context);
void usb_dev_priv_init(void * context);

void usb_dev_default_event(void * context);
int usb_dev_default_if_req(void * context, int event);

char htoc(int nibble);

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#endif /* USB_DEVICE_H_ */

/*! @} */
