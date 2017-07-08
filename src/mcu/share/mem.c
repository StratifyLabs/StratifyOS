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
#include "mcu/mem.h"


//These functions are device specific
extern void _mcu_mem_dev_power_on(int port);
extern void _mcu_mem_dev_power_off(int port);
extern int _mcu_mem_dev_powered_on(int port);
extern int _mcu_mem_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop);
extern int _mcu_mem_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop);

int (* const mem_ioctl_func_table[I_MCU_TOTAL + I_MEM_TOTAL])(int, void*) = {
		mcu_mem_getinfo,
		mcu_mem_setattr,
		mcu_mem_setaction,
		mcu_mem_erasepage,
		mcu_mem_getpageinfo,
		mcu_mem_writepage
};

int mcu_mem_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			_mcu_mem_dev_powered_on,
			_mcu_mem_dev_power_on);
}

int mcu_mem_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			_mcu_mem_dev_powered_on,
			mem_ioctl_func_table,
			I_MCU_TOTAL + I_MEM_TOTAL);
}

int mcu_mem_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_read(cfg, rop,
			_mcu_mem_dev_powered_on,
			_mcu_mem_dev_read);
}


int mcu_mem_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return mcu_write(cfg, wop,
			_mcu_mem_dev_powered_on,
			_mcu_mem_dev_write);

}

int mcu_mem_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, _mcu_mem_dev_powered_on, _mcu_mem_dev_power_off);
}


