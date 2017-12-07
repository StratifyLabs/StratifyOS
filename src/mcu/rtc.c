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
#include "mcu/rtc.h"

//These functions are device specific
extern void mcu_rtc_dev_power_on(const devfs_handle_t * handle);
extern void mcu_rtc_dev_power_off(const devfs_handle_t * handle);
extern int mcu_rtc_dev_is_powered(const devfs_handle_t * handle);
static int get_version(const devfs_handle_t * handle, void* ctl){
	return RTC_VERSION;
}

int (* const rtc_ioctl_func_table[I_MCU_TOTAL + I_RTC_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_rtc_getinfo,
		mcu_rtc_setattr,
		mcu_rtc_setaction,
		mcu_rtc_set,
		mcu_rtc_get
};

int mcu_rtc_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			mcu_rtc_dev_is_powered,
			mcu_rtc_dev_power_on);
}

int mcu_rtc_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			mcu_rtc_dev_is_powered,
			rtc_ioctl_func_table,
			I_MCU_TOTAL + I_RTC_TOTAL);
}

int mcu_rtc_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int mcu_rtc_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int mcu_rtc_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, mcu_rtc_dev_is_powered, mcu_rtc_dev_power_off);
}


