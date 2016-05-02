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
#include "mcu/gpio.h"



//These functions are device specific
extern void _mcu_gpio_dev_power_on(int port);
extern void _mcu_gpio_dev_power_off(int port);
extern int _mcu_gpio_dev_powered_on(int port);

int (* const gpio_ioctl_func_table[I_GLOBAL_TOTAL + I_GPIO_TOTAL])(int, void*) = {
		mcu_gpio_getattr,
		mcu_gpio_setattr,
		mcu_gpio_setaction,
		mcu_gpio_setmask,
		mcu_gpio_clrmask,
		mcu_gpio_get,
		mcu_gpio_set,
		mcu_gpio_setpinmode
};

int mcu_gpio_open(const device_cfg_t * cfg){
	return mcu_open(cfg,
			MCU_GPIO_PORTS,
			_mcu_gpio_dev_powered_on,
			_mcu_gpio_dev_power_on);
}

int mcu_gpio_ioctl(const device_cfg_t * cfg, int request, void * ctl){

	return mcu_ioctl(cfg,
			request,
			ctl,
			MCU_GPIO_PORTS,
			_mcu_gpio_dev_powered_on,
			gpio_ioctl_func_table,
			I_GLOBAL_TOTAL + I_GPIO_TOTAL);
}

int mcu_gpio_read(const device_cfg_t * cfg, device_transfer_t * rop){
	errno = ENOTSUP;
	return -1;

}


int mcu_gpio_write(const device_cfg_t * cfg, device_transfer_t * wop){
	errno = ENOTSUP;
	return -1;

}

int mcu_gpio_close(const device_cfg_t * cfg){
	return mcu_close(cfg, MCU_GPIO_PORTS, _mcu_gpio_dev_powered_on, _mcu_gpio_dev_power_off);
}


