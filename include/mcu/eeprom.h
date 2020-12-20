// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup EEPROM EEPROM (EE)
 *
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The EEPROM module has functions to access the on chip EEPROM.
 *
 *
 */

/*! \file
 * \brief EEPROM Header file
 */

#ifndef DEV_EEPROM_H_
#define DEV_EEPROM_H_

#include "sos/dev/eeprom.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int mcu_eeprom_open(const devfs_handle_t * handle);
int mcu_eeprom_read(const devfs_handle_t * handle, devfs_async_t * rop);
int mcu_eeprom_write(const devfs_handle_t * handle, devfs_async_t * wop);
int mcu_eeprom_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int mcu_eeprom_close(const devfs_handle_t * handle);

int mcu_eeprom_getinfo(const devfs_handle_t * handle, void * ctl);
int mcu_eeprom_setattr(const devfs_handle_t * handle, void * ctl);
int mcu_eeprom_setaction(const devfs_handle_t * handle, void * ctl);


#ifdef __cplusplus
}
#endif

#endif // EEPROM_HEADER

/*! @} */
