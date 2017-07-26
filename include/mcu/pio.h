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

/*! \addtogroup PIO_DEV PIO
 *
 * \ingroup DEV
 *
 */

#ifndef _MCU_PIO_H_
#define _MCU_PIO_H_

#include "core.h"
#include "sos/dev/pio.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif


int mcu_pio_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_pio_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_pio_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_pio_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_pio_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;

int mcu_pio_getinfo(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_pio_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_pio_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;

int mcu_pio_setmask(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_pio_clrmask(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_pio_get(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_pio_set(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;


#ifndef __link

static inline int mcu_i2c_cfg_pio(const devfs_handle_t * handle, int pio_port, int scl_pin, int sda_pin) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_i2c_cfg_pio(const devfs_handle_t * handle, int pio_port, int scl_pin, int sda_pin){
	if ( mcu_core_set_pinsel_func(pio_port, sda_pin, CORE_PERIPH_I2C, handle->port) ) return -1;  //Use pins for I2C
	if ( mcu_core_set_pinsel_func(pio_port, scl_pin, CORE_PERIPH_I2C, handle->port) ) return -1;  //Use pins for I2C
	return 0;
}


static inline int mcu_uart_cfg_pio(const devfs_handle_t * handle, int pio_port, int rx_pin, int tx_pin) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_uart_cfg_pio(const devfs_handle_t * handle, int pio_port, int rx_pin, int tx_pin){
	if ( mcu_core_set_pinsel_func(pio_port, rx_pin, CORE_PERIPH_UART, handle->port) ) return -1;  //Use pin for UART
	if ( mcu_core_set_pinsel_func(pio_port, tx_pin, CORE_PERIPH_UART, handle->port) ) return -1;  //Use pin for UART
	return 0;
}

static inline int mcu_spi_cfg_pio(const devfs_handle_t * handle, int pio_port, int mosi_pin, int miso_pin, int sck_pin) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_spi_cfg_pio(const devfs_handle_t * handle, int pio_port, int mosi_pin, int miso_pin, int sck_pin){
	if ( mcu_core_set_pinsel_func(pio_port, mosi_pin, CORE_PERIPH_SPI, handle->port) ) return -1;  //Use pin for SPI
	if ( mcu_core_set_pinsel_func(pio_port, miso_pin, CORE_PERIPH_SPI, handle->port) ) return -1;  //Use pin for SPI
	if ( mcu_core_set_pinsel_func(pio_port, sck_pin, CORE_PERIPH_SPI, handle->port) ) return -1;  //Use pin for SPI
	return 0;
}

static inline int mcu_ssp_cfg_pio(const devfs_handle_t * handle, int pio_port, int mosi_pin, int miso_pin, int sck_pin) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_ssp_cfg_pio(const devfs_handle_t * handle, int pio_port, int mosi_pin, int miso_pin, int sck_pin){
	if ( mcu_core_set_pinsel_func(pio_port, mosi_pin, CORE_PERIPH_SSP, handle->port) ) return -1;  //Use pin for SPI
	if ( mcu_core_set_pinsel_func(pio_port, miso_pin, CORE_PERIPH_SSP, handle->port) ) return -1;  //Use pin for SPI
	if ( mcu_core_set_pinsel_func(pio_port, sck_pin, CORE_PERIPH_SSP, handle->port) ) return -1;  //Use pin for SPI
	return 0;
}



#endif

#ifdef __cplusplus
}
#endif



#endif /* _MCU_PIO_H_ */

/*! @} */
