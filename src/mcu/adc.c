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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include <mcu/local.h>
#include "mcu/adc.h"

//These functions are device specific
extern void mcu_adc_dev_power_on(const devfs_handle_t * handle) MCU_ROOT_CODE;
extern void mcu_adc_dev_power_off(const devfs_handle_t * handle) MCU_ROOT_CODE;
extern int mcu_adc_dev_is_powered(const devfs_handle_t * handle) MCU_ROOT_CODE;
extern int mcu_adc_dev_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
static int get_version(const devfs_handle_t * handle, void* ctl){
	return ADC_VERSION;
}

int (* const adc_ioctl_func_table[I_MCU_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_adc_getinfo,
		mcu_adc_setattr,
		mcu_adc_setaction
};

int mcu_adc_open(const devfs_handle_t * handle){
	return mcu_open(handle,
			mcu_adc_dev_is_powered,
			mcu_adc_dev_power_on);
}

int mcu_adc_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	return mcu_ioctl(handle,
			request,
			ctl,
			mcu_adc_dev_is_powered,
			adc_ioctl_func_table,
			I_MCU_TOTAL);
}

int mcu_adc_read(const devfs_handle_t * handle, devfs_async_t * rop){
	return mcu_read(handle,
			rop,
			mcu_adc_dev_is_powered,
			mcu_adc_dev_read);
}

int mcu_adc_write(const devfs_handle_t * handle, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int mcu_adc_close(const devfs_handle_t * handle){
	return mcu_close(handle, mcu_adc_dev_is_powered, mcu_adc_dev_power_off);
}


