// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup QSPI_DEV QSPI
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_QSPI_H_
#define _MCU_QSPI_H_

#include "sos/dev/qspi.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} qspi_event_data_t;

typedef struct MCU_PACK {
  qspi_attr_t attr; // default attributes
  u32 port;
} qspi_config_t;

#define MCU_QSPI_IOCTL_REQUEST_DECLARATION(driver_name)                                  \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);                         \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, execcommand)

#define MCU_QSPI_DRIVER_DECLARATION(variant)                                             \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_QSPI_IOCTL_REQUEST_DECLARATION(variant)

MCU_QSPI_DRIVER_DECLARATION(mcu_qspi);
MCU_QSPI_DRIVER_DECLARATION(mcu_qspi_dma);

#ifdef __cplusplus
}
#endif

#endif /* _MCU_QSPI_H_ */

/*! @} */
