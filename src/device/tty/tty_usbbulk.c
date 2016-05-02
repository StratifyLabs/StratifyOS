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
#include "iface/dev/tty.h"


int tty_usbbulk_open(const device_cfg_t * cfg){
	return 0;
}

int tty_usbbulk_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	return mcu_usb_ioctl(cfg, request, ctl);
}

int tty_usbbulk_write(const device_cfg_t * cfg, device_transfer_t * wop){
	const tty_cfg_t * dcfg;
	dcfg = (const tty_cfg_t*)cfg->dcfg;
	wop->loc = dcfg->write_ep;
	return mcu_usb_write(cfg, wop);
}

int tty_usbbulk_read(const device_cfg_t * cfg, device_transfer_t * rop){ //The stdio is read by the USB connection
	const tty_cfg_t * dcfg;
	dcfg = (const tty_cfg_t*)cfg->dcfg;
	rop->loc = dcfg->read_ep;
	return mcu_usb_read(cfg, rop);
}

int tty_usbbulk_close(const device_cfg_t * cfg){
	return 0;
}
