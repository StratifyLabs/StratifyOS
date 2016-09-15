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


#include "../iface/dev/spi.h"
#include "arch.h"
#include "iface/device_config.h"

#ifdef __cplusplus
extern "C" {
#endif


/*! \details
 * \sa periph_open()
 *
 */
int mcu_spi_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_spi_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_spi_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_spi_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_spi_close(const device_cfg_t * cfg) MCU_PRIV_CODE;

int mcu_spi_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_spi_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_spi_setaction(int port, void * ctl) MCU_PRIV_CODE;
int mcu_spi_swap(int port, void * ctl) MCU_PRIV_CODE;
int mcu_spi_setduplex(int port, void * ctl) MCU_PRIV_CODE;


int mcu_ssp_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
int mcu_ssp_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
int mcu_ssp_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
int mcu_ssp_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_close(const device_cfg_t * cfg) MCU_PRIV_CODE;
int mcu_ssp_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setaction(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_swap(int port, void * ctl) MCU_PRIV_CODE;
int mcu_ssp_setduplex(int port, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif /* _MCU_SPI_H_ */

/*! @} */
