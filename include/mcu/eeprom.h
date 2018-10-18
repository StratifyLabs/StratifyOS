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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

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
