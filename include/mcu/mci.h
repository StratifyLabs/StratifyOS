// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SPI_DEV SPI
 * @{
 *
 * \ingroup DEV
 */

#ifndef mcu_mci_H_
#define mcu_mci_H_


#include "sos/dev/mci.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int mcu_mci_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
int mcu_mci_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_mci_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_mci_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int mcu_mci_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_mci_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_mci_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_mci_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif


#endif /* mcu_mci_H_ */

/*! @} */
