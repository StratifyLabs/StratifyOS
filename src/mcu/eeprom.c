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

#include <mcu/local.h>
#include "mcu/eeprom.h"

#define I_EEPROM_TOTAL 0

//These functions are device specific
extern void mcu_eeprom_dev_power_on(const devfs_handle_t * handle);
extern void mcu_eeprom_dev_power_off(const devfs_handle_t * handle);
extern int mcu_eeprom_dev_is_powered(const devfs_handle_t * handle);
extern int mcu_eeprom_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop);
extern int mcu_eeprom_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop);
static int get_version(const devfs_handle_t * handle, void* ctl){
	return EEPROM_VERSION;
}

int (* const eeprom_ioctl_func_table[I_MCU_TOTAL + I_EEPROM_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_eeprom_getinfo,
		mcu_eeprom_setattr,
		mcu_eeprom_setaction
};

int mcu_eeprom_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			mcu_eeprom_dev_is_powered,
			mcu_eeprom_dev_power_on);
}

int mcu_eeprom_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			mcu_eeprom_dev_is_powered,
			eeprom_ioctl_func_table,
			I_MCU_TOTAL + I_EEPROM_TOTAL);
}




int mcu_eeprom_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_read(cfg, rop,
			mcu_eeprom_dev_is_powered,
			mcu_eeprom_dev_read);
}


int mcu_eeprom_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return mcu_write(cfg, wop,
			mcu_eeprom_dev_is_powered,
			mcu_eeprom_dev_write);
}

int mcu_eeprom_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, mcu_eeprom_dev_is_powered, mcu_eeprom_dev_power_off);
}


