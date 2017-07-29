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

#include "sos/dev/adc.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	adc_attr_t attr; //default attributes
	u32 reference_mv;
} adc_config_t;

int mcu_adc_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_adc_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_adc_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_adc_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_adc_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;


int mcu_adc_getinfo(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_adc_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_adc_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;

int mcu_adc_dma_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_adc_dma_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_adc_dma_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_adc_dma_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
int mcu_adc_dma_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;


int mcu_adc_dma_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_adc_dma_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;


#ifdef __cplusplus
}
#endif


#endif // ADC_H_

/*! @} */

