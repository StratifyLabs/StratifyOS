// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SD_DEV SDIO
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_SDIO_H_
#define _MCU_SDIO_H_

#include "sos/dev/sdio.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} sdio_event_data_t;

typedef struct MCU_PACK {
  sdio_attr_t attr; // default attributes
  u32 port;
} sdio_config_t;

#define MCU_SDIO_IOCTL_REQUEST_DECLARATION(driver_name)                                  \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);                         \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getcid);                            \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getcsd);                            \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getstatus)

#define MCU_SDIO_DRIVER_DECLARATION(variant)                                             \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_SDIO_IOCTL_REQUEST_DECLARATION(variant)

MCU_SDIO_DRIVER_DECLARATION(mcu_sdio);
MCU_SDIO_DRIVER_DECLARATION(mcu_sdio_dma);

#ifdef __cplusplus
}
#endif

#endif /* _MCU_SDIO_H_ */

/*! @} */
