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
#include "mcu/qei.h"

//These functions are device specific
extern void mcu_qei_dev_power_on(const devfs_handle_t * handle);
extern void mcu_qei_dev_power_off(const devfs_handle_t * handle);
extern int mcu_qei_dev_is_powered(const devfs_handle_t * handle);
static int get_version(const devfs_handle_t * handle, void* ctl){
	return QEI_VERSION;
}

int (* const qei_ioctl_func_table[I_MCU_TOTAL + I_QEI_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_qei_getinfo,
		mcu_qei_setattr,
		mcu_qei_setaction,
		mcu_qei_get,
		mcu_qei_getvelocity,
		mcu_qei_getindex
};

int mcu_qei_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			mcu_qei_dev_is_powered,
			mcu_qei_dev_power_on);
}

int mcu_qei_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			mcu_qei_dev_is_powered,
			qei_ioctl_func_table,
			I_MCU_TOTAL + I_QEI_TOTAL);
}



int mcu_qei_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;

}

int mcu_qei_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;

}

int mcu_qei_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, mcu_qei_dev_is_powered, mcu_qei_dev_power_off);
}


