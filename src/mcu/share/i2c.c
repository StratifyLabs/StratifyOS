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
#include "mcu/i2c.h"

//These functions are device specific
extern void mcu_i2c_dev_power_on(const devfs_handle_t * handle);
extern void mcu_i2c_dev_power_off(const devfs_handle_t * handle);
extern int mcu_i2c_dev_is_powered(const devfs_handle_t * handle);
extern int mcu_i2c_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop);
extern int mcu_i2c_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop);
static int get_version(const devfs_handle_t * handle, void* ctl){
	return I2C_VERSION;
}

int (* const i2c_ioctl_func_table[I_MCU_TOTAL + I_I2C_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_i2c_getinfo,
		mcu_i2c_setattr,
		mcu_i2c_setaction,
};

int mcu_i2c_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			mcu_i2c_dev_is_powered,
			mcu_i2c_dev_power_on);
}

int mcu_i2c_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			mcu_i2c_dev_is_powered,
			i2c_ioctl_func_table,
			I_MCU_TOTAL + I_I2C_TOTAL);
}



int mcu_i2c_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_read(cfg, rop,
			mcu_i2c_dev_is_powered,
			mcu_i2c_dev_read);

}


int mcu_i2c_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return mcu_write(cfg, wop,
			mcu_i2c_dev_is_powered,
			mcu_i2c_dev_write);

}

int mcu_i2c_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, mcu_i2c_dev_is_powered, mcu_i2c_dev_power_off);
}



