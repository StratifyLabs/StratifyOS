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
#include "mcu/pio.h"



//These functions are device specific
extern void _mcu_pio_dev_power_on(int port);
extern void _mcu_pio_dev_power_off(int port);
extern int _mcu_pio_dev_powered_on(int port);
extern int _mcu_pio_dev_write(const device_cfg_t * cfg, device_transfer_t * wop);

int (* const _mcu_pio_ioctl_func_table[I_GLOBAL_TOTAL + I_PIO_TOTAL])(int, void*) = {
		mcu_pio_getattr,
		mcu_pio_setattr,
		mcu_pio_setaction,
		mcu_pio_setmask,
		mcu_pio_clrmask,
		mcu_pio_get,
		mcu_pio_set
};

int mcu_pio_open(const device_cfg_t * cfg){
	return mcu_open(cfg,
			MCU_PIO_PORTS,
			_mcu_pio_dev_powered_on,
			_mcu_pio_dev_power_on);
}

int mcu_pio_ioctl(const device_cfg_t * cfg, int request, void * ctl){

	return mcu_ioctl(cfg,
			request,
			ctl,
			MCU_PIO_PORTS,
			_mcu_pio_dev_powered_on,
			_mcu_pio_ioctl_func_table,
			I_GLOBAL_TOTAL + I_PIO_TOTAL);
}

int mcu_pio_read(const device_cfg_t * cfg, device_transfer_t * rop){
	errno = ENOTSUP;
	return -1;

}


int mcu_pio_write(const device_cfg_t * cfg, device_transfer_t * wop){
	return mcu_write(cfg,
			wop,
			MCU_PIO_PORTS,
			_mcu_pio_dev_powered_on,
			_mcu_pio_dev_write);
}

int mcu_pio_close(const device_cfg_t * cfg){
	return mcu_close(cfg, MCU_PIO_PORTS, _mcu_pio_dev_powered_on, _mcu_pio_dev_power_off);
}


