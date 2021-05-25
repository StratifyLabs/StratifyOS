// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SD_DEV MMC
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_MMC_H_
#define _MCU_MMC_H_

#include "sos/dev/mmc.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} mmc_event_data_t;

typedef struct MCU_PACK {
  mmc_attr_t attr; // default attributes
  u32 port;
} mmc_config_t;

#define MCU_MMC_IOCTL_REQUEST_DECLARATION(driver_name)                                   \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);                         \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getcid);                            \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getcsd);                            \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getstatus)

#define MCU_MMC_DRIVER_DECLARATION(variant)                                              \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_MMC_IOCTL_REQUEST_DECLARATION(variant)

MCU_MMC_DRIVER_DECLARATION(mcu_mmc);
MCU_MMC_DRIVER_DECLARATION(mcu_mmc_dma);

#ifdef __cplusplus
}
#endif

#endif /* _MCU_MMC_H_ */

/*! @} */
