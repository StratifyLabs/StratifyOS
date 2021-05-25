// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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
  adc_attr_t attr; // default attributes
  u32 port;
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
