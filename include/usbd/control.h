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

/*! \addtogroup USB_DEV USB Device
 *
 * @{
 *
 */

#ifndef USBD_CONTROL_H_
#define USBD_CONTROL_H_

#include "sos/fs/devfs.h"
#include "sos/dev/usb.h"
#include "mcu/types.h"

#include "defines.h"
#include "types.h"

//#define USBD_PORT 0
#define USBD_ALT_SETTING_SIZE 16


typedef struct {
	const devfs_handle_t handle;
	const void * const device;
	const void * const config;
	const void * const string;
	int (* const class_event_handler)(void * context, const mcu_event_t * event);
} usbd_control_constants_t;

#define MCU_CORE_USB_MAX_PACKET_ZERO_VALUE 64

#define USBD_CONTROL_DATAOUT_BUF_SIZE 256

typedef struct {
  u8 * dptr;
  u16 nbyte;
  u16 is_zlp;
} usbd_control_data_t;

typedef struct MCU_PACK {
	usbd_setup_packet_t setup_packet;
	usbd_control_data_t data;
	u16 status;
	u8 addr;
	u8 current_configuration;
	u32 ep_mask;
	u32 ep_halt;
	u32 ep_stall;
	u8 num_interfaces;
	u8 buf[USBD_CONTROL_DATAOUT_BUF_SIZE];
	u8 alt_setting[USBD_ALT_SETTING_SIZE];
	const usbd_control_constants_t * constants;
	const devfs_handle_t * handle;
} usbd_control_t;

void usbd_control_svcall_init(void * context);
void usbd_control_root_init(void * context);
int usbd_control_handler(void * context, const mcu_event_t * data);

void usbd_control_handler_setup_stage(usbd_control_t * context);
void usbd_control_datain_stage(usbd_control_t * context) MCU_NEVER_INLINE;

void usbd_control_dataout_stage (usbd_control_t * context);
void usbd_control_statusin_stage(usbd_control_t * context);
void usbd_control_statusout_stage (usbd_control_t * context);

static inline int usbd_control_setup_request_type(usbd_control_t * context){
	return context->setup_packet.bmRequestType.bitmap_t.type;
}

static inline int usbd_control_setup_interface(usbd_control_t * context){
	return context->setup_packet.wIndex.b[0];
}

static inline u16 usbd_control_setup_index(usbd_control_t * context){
    return context->setup_packet.wIndex.w;
}

static inline u16 usbd_control_setup_value(usbd_control_t * context){
    return context->setup_packet.wValue.w;
}

static inline int usbd_control_setup_request(usbd_control_t * context){
	return context->setup_packet.bRequest;
}

static inline int usbd_control_setup_length(usbd_control_t * context){
    return context->setup_packet.wLength;
}

static inline int usbd_control_setup_request_direction(usbd_control_t * context){
	return context->setup_packet.bmRequestType.bitmap_t.dir;
}

static inline int usbd_control_setup_request_recipient(usbd_control_t * context){
    return context->setup_packet.bmRequestType.bitmap_t.recipient;
}

static inline void usbd_control_prepare_buffer(usbd_control_t * context){
	context->data.dptr = context->buf;
}


char htoc(int nibble);

int usbd_control_reset(const devfs_handle_t * handle);
int usbd_control_attach(const devfs_handle_t * handle);
int usbd_control_detach(const devfs_handle_t * handle);
int usbd_control_configure(const devfs_handle_t * handle);
int usbd_control_unconfigure(const devfs_handle_t * handle);
int usbd_control_set_address(const devfs_handle_t * handle, u8 address);
int usbd_control_reset_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_enable_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_disable_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_stall_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_unstall_endpoint(const devfs_handle_t * handle, u8 endpoint_number);
int usbd_control_configure_endpoint(const devfs_handle_t * handle, const usbd_endpoint_descriptor_t * endpoint_descriptor);




#endif /* USBD_CONTROL_H_ */

/*! @} */
