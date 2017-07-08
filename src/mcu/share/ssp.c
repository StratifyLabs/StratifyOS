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
#include "mcu/spi.h"

int mcu_ssp_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_ssp_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_ssp_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_ssp_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_ssp_getinfo(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setaction(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_swap(int port, void * ctl) MCU_PRIV_CODE;

//These functions are device specific
extern void _mcu_ssp_dev_power_on(int port);
extern void _mcu_ssp_dev_power_off(int port);
extern int _mcu_ssp_dev_powered_on(int port);
extern int _mcu_ssp_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop);
extern int _mcu_ssp_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop);

int (* const ssp_ioctl_func_table[I_MCU_TOTAL + I_SPI_TOTAL])(int, void*) = {
		mcu_ssp_getinfo,
		mcu_ssp_setattr,
		mcu_ssp_setaction,
		mcu_ssp_swap
};

int mcu_ssp_open(const devfs_handle_t * cfg){
	return mcu_open(cfg,
			_mcu_ssp_dev_powered_on,
			_mcu_ssp_dev_power_on);
}


int mcu_ssp_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			_mcu_ssp_dev_powered_on,
			ssp_ioctl_func_table,
			I_MCU_TOTAL + I_SPI_TOTAL);
}



int mcu_ssp_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return mcu_read(cfg, rop,
			_mcu_ssp_dev_powered_on,
			_mcu_ssp_dev_read);

}


int mcu_ssp_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return mcu_write(cfg, wop,
			_mcu_ssp_dev_powered_on,
			_mcu_ssp_dev_write);

}

int mcu_ssp_close(const devfs_handle_t * cfg){
	return mcu_close(cfg, _mcu_ssp_dev_powered_on, _mcu_ssp_dev_power_off);
}


