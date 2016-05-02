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

/*! \addtogroup ADC_DEV ADC
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_ADC_H_
#define _MCU_ADC_H_

#include "../iface/dev/adc.h"
#include "iface/device_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \details
 * \sa periph_open()
 *
 */
int mcu_adc_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_adc_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_adc_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_adc_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_adc_close(const device_cfg_t * cfg) MCU_PRIV_CODE;


int mcu_adc_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_adc_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_adc_setaction(int port, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif // ADC_H_

/*! @} */

