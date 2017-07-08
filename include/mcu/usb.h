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

/*! \addtogroup USB_DEV USB
 * @{
 * \ingroup DEV
 *
 */

#ifndef _MCU_USB_H_
#define _MCU_USB_H_

#include "sos/dev/usb.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 value;
} usb_event_data_t;

int mcu_usb_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_usb_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_usb_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_usb_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_usb_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;


int mcu_usb_getinfo(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_setaction(int port, void * ctl) MCU_PRIV_CODE;

int mcu_usb_reset(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_attach(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_detach(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_configure(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_setaddr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_resetep(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_enableep(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_disableep(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_stallep(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_unstallep(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_cfgep(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_seteventhandler(int port, void * ctl) MCU_PRIV_CODE;
int mcu_usb_isconnected(int port, void * ctl) MCU_PRIV_CODE;


//Endpoint functions
int mcu_usb_rd_ep(int port, u32 endpoint_num, void * dest) MCU_PRIV_CODE;
int mcu_usb_wr_ep(int port, u32 endpoint_num, const void * src, u32 size) MCU_PRIV_CODE;


#ifdef __cplusplus
}
#endif

#endif /* _MCU_USB_H_ */

/*! @} */
