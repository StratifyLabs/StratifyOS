/* Copyright 2011 MCU_PRIV_CODE; 
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

/*! \addtogroup SPI_DEV SPI
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_SPI_H_
#define _MCU_SPI_H_

#include "sos/dev/spi.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 value;
} spi_event_data_t;

typedef struct MCU_PACK {
	spi_attr_t attr; //default attributes
} spi_config_t;

int mcu_spi_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_spi_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_spi_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_spi_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_spi_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;

int mcu3_spi_open(const devfs_handle_t * handle) MCU_PRIV_CODE;
int mcu3_spi_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu3_spi_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu3_spi_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_PRIV_CODE;
int mcu3_spi_close(const devfs_handle_t * handle) MCU_PRIV_CODE;

int mcu_spi_getinfo(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_spi_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_spi_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_spi_swap(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;


int mcu_ssp_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_ssp_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_ssp_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_ssp_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_ssp_getinfo(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_swap(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif /* _MCU_SPI_H_ */

/*! @} */
