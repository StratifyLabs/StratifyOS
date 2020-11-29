/* Copyright 2011-2018 Tyler Gilbert;
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
  u32 value;
} adc_event_t;

typedef struct MCU_PACK {
  u32 port;
  adc_attr_t attr; // default attributes
  u32 reference_mv;
} adc_config_t;

#define MCU_ADC_IOCTL_REQUEST_DECLARATION(driver_name)                                   \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction)

#define MCU_ADC_DRIVER_DECLARATION(variant)                                              \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_ADC_IOCTL_REQUEST_DECLARATION(variant)

MCU_ADC_DRIVER_DECLARATION(mcu_adc);
MCU_ADC_DRIVER_DECLARATION(mcu_adc_dma);

#ifdef __cplusplus
}
#endif

#endif // ADC_H_

/*! @} */
