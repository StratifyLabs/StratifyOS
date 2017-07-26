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
extern void flash_dev_power_on(const devfs_handle_t * handle);
extern void flash_dev_power_off(const devfs_handle_t * handle);
extern int flash_dev_is_powered(const devfs_handle_t * handle);
extern int mcu_flash_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop);
extern int mcu_flash_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop);

int (* const flash_ioctl_func_table[I_MCU_TOTAL + I_FLASH_TOTAL])(const devfs_handle_t*, void*) = {
		mcu_flash_getinfo,
		mcu_flash_setattr,
		mcu_flash_setaction,
		mcu_flash_eraseaddr,
		mcu_flash_erasepage,
		mcu_flash_getpage,
		mcu_flash_getsize,
		mcu_flash_getpageinfo,
		mcu_flash_writepage
};

int mcu_flash_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			flash_dev_is_powered,
			flash_dev_power_on);
}

int mcu_flash_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			flash_dev_is_powered,
			flash_ioctl_func_table,
			I_MCU_TOTAL + I_FLASH_TOTAL);
}




int mcu_flash_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_read(cfg, rop,
			flash_dev_is_powered,
			mcu_flash_dev_read);
}


int mcu_flash_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int mcu_flash_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, flash_dev_is_powered, flash_dev_power_off);
}


