// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup ENET Ethernet
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details
 *
 */

/*! \file
 * \brief Ethernet Header File
 *
 */

#ifndef MCU_ETH_H_
#define MCU_ETH_H_

#include "sos/dev/eth.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  eth_attr_t attr; // default attributes
  u32 port;
} eth_config_t;

int mcu_eth_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_eth_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_eth_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_eth_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;

int mcu_eth_close(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_eth_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_eth_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_eth_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_eth_setregister(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_eth_getregister(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* MCU_ETH_H_ */

/*! @} */
