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
#include "mcu/emc.h"


//These functions are device specific
extern void mcu_emc_dev_power_on(const devfs_handle_t * handle);
extern void mcu_emc_dev_power_off(const devfs_handle_t * handle);
extern int mcu_emc_dev_is_powered(const devfs_handle_t * handle);
static int get_version(const devfs_handle_t * handle, void* ctl){
	return EMC_VERSION;
}

int (* const emc_ioctl_func_table[I_MCU_TOTAL + I_EMC_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_emc_getinfo,
		mcu_emc_setattr,
		mcu_emc_setaction
};

int mcu_emc_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			mcu_emc_dev_is_powered,
			mcu_emc_dev_power_on);
}

int mcu_emc_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			mcu_emc_dev_is_powered,
			emc_ioctl_func_table,
			I_MCU_TOTAL + I_EMC_TOTAL);
}

int mcu_emc_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int mcu_emc_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int mcu_emc_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, mcu_emc_dev_is_powered, mcu_emc_dev_power_off);
}


