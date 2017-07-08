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

#include <errno.h>
#include "local.h"
#include "mcu/tmr.h"

//These functions are device specific
extern void _mcu_tmr_dev_power_on(int port);
extern void _mcu_tmr_dev_power_off(int port);
extern int _mcu_tmr_dev_powered_on(int port);

static int invalid_request(int port, void * ctl){
	errno = ENOTSUP;
	return -1;
}

int (* const tmrsimple_ioctl_func_table[I_MCU_TOTAL + I_TMR_TOTAL])(int, void*) = {
		mcu_tmrsimple_getinfo,
		mcu_tmrsimple_setattr,
		mcu_tmrsimple_setaction,
		invalid_request,
		invalid_request,
		invalid_request,
		invalid_request,
		mcu_tmr_set,
		mcu_tmr_get
};


int mcu_tmrsimpleopen(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			_mcu_tmr_dev_powered_on,
			_mcu_tmr_dev_power_on);
}

int mcu_tmrsimple_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			_mcu_tmr_dev_powered_on,
			tmrsimple_ioctl_func_table,
			I_MCU_TOTAL + I_TMR_TOTAL);
}

int mcu_tmrsimple_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int mcu_tmrsimple_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int mcu_tmrsimple_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, _mcu_tmr_dev_powered_on, _mcu_tmr_dev_power_off);
}


