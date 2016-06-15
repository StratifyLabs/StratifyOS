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

#if MCU_SPI_PORTS > 0

//These functions are device specific
extern void _mcu_spi_dev_power_on(int port);
extern void _mcu_spi_dev_power_off(int port);
extern int _mcu_spi_dev_powered_on(int port);
extern int _mcu_spi_dev_read(const device_cfg_t * cfg, device_transfer_t * rop);
extern int _mcu_spi_dev_write(const device_cfg_t * cfg, device_transfer_t * wop);


int (* const spi_ioctl_func_table[I_GLOBAL_TOTAL + I_SPI_TOTAL])(int, void*) = {
		mcu_spi_getattr,
		mcu_spi_setattr,
		mcu_spi_setaction,
		mcu_spi_swap,
		mcu_spi_setduplex,
};

int mcu_spi_open(const device_cfg_t * cfg){
	return mcu_open(cfg,
			MCU_SPI_PORTS,
			_mcu_spi_dev_powered_on,
			_mcu_spi_dev_power_on);
}

int mcu_spi_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	return mcu_ioctl(cfg,
			request,
			ctl,
			MCU_SPI_PORTS,
			_mcu_spi_dev_powered_on,
			spi_ioctl_func_table,
			I_GLOBAL_TOTAL + I_SPI_TOTAL);
}



int mcu_spi_read(const device_cfg_t * cfg, device_transfer_t * rop){
	return mcu_read(cfg, rop,
			MCU_SPI_PORTS,
			_mcu_spi_dev_powered_on,
			_mcu_spi_dev_read);

}


int mcu_spi_write(const device_cfg_t * cfg, device_transfer_t * wop){
	return mcu_write(cfg, wop,
			MCU_SPI_PORTS,
			_mcu_spi_dev_powered_on,
			_mcu_spi_dev_write);

}

int mcu_spi_close(const device_cfg_t * cfg){
	return mcu_close(cfg, MCU_SPI_PORTS, _mcu_spi_dev_powered_on, _mcu_spi_dev_power_off);
}

#endif


