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

#include "local.h"
#include "mcu/flash.h"


//These functions are device specific
extern void flash_dev_power_on(int port);
extern void flash_dev_power_off(int port);
extern int flash_dev_powered_on(int port);
extern int _mcu_flash_dev_read(const device_cfg_t * cfg, device_transfer_t * rop);
extern int _mcu_flash_dev_write(const device_cfg_t * cfg, device_transfer_t * wop);

int (* const flash_ioctl_func_table[I_GLOBAL_TOTAL + I_FLASH_TOTAL])(int, void*) = {
		mcu_flash_getattr,
		mcu_flash_setattr,
		mcu_flash_setaction,
		mcu_flash_eraseaddr,
		mcu_flash_erasepage,
		mcu_flash_getpage,
		mcu_flash_getsize,
		mcu_flash_getpageinfo,
		mcu_flash_writepage
};

int mcu_flash_open(const device_cfg_t * cfg){
	return mcu_open(cfg,
			MCU_FLASH_PORTS,
			flash_dev_powered_on,
			flash_dev_power_on);
}

int mcu_flash_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			MCU_FLASH_PORTS,
			flash_dev_powered_on,
			flash_ioctl_func_table,
			I_GLOBAL_TOTAL + I_FLASH_TOTAL);
}




int mcu_flash_read(const device_cfg_t * cfg, device_transfer_t * rop){
	return mcu_read(cfg, rop,
			MCU_FLASH_PORTS,
			flash_dev_powered_on,
			_mcu_flash_dev_read);
}


int mcu_flash_write(const device_cfg_t * cfg, device_transfer_t * wop){
	errno = ENOTSUP;
	return -1;
}

int mcu_flash_close(const device_cfg_t * cfg){
	return mcu_close(cfg, MCU_FLASH_PORTS, flash_dev_powered_on, flash_dev_power_off);
}


