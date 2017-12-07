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
#include "mcu/enet.h"

//These functions are device specific
extern void mcu_enet_dev_power_on(const devfs_handle_t * handle);
extern void mcu_enet_dev_power_off(const devfs_handle_t * handle);
extern int mcu_enet_dev_is_powered(const devfs_handle_t * handle);
extern int mcu_enet_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop);
extern int mcu_enet_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop);
static int get_version(const devfs_handle_t * handle, void* ctl){
	return ENET_VERSION;
}

int (* const enet_ioctl_func_table[I_MCU_TOTAL + I_ENET_TOTAL])(const devfs_handle_t*, void*) = {
		get_version,
		mcu_enet_getinfo,
		mcu_enet_setattr,
		mcu_enet_setaction,
		mcu_enet_inittxpkt,
};

int mcu_enet_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			mcu_enet_dev_is_powered,
			mcu_enet_dev_power_on);
}

int mcu_enet_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			mcu_enet_dev_is_powered,
			enet_ioctl_func_table,
			I_MCU_TOTAL + I_ENET_TOTAL);
}



int mcu_enet_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_read(cfg, rop,
			mcu_enet_dev_is_powered,
			mcu_enet_dev_read);
}

int mcu_enet_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return mcu_write(cfg, wop,
			mcu_enet_dev_is_powered,
			mcu_enet_dev_write);

}

int mcu_enet_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, mcu_enet_dev_is_powered, mcu_enet_dev_power_off);
}


