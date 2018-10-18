/* Copyright 2013; 
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

/*! \addtogroup EINT_DEV EINT
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_EMC_H_
#define _MCU_EMC_H_


#include "sos/dev/emc.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \details
 * \sa periph_open()
 *
 */
int mcu_emc_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_emc_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_emc_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_emc_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_emc_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_emc_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_emc_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_emc_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;


#ifdef __cplusplus
}
#endif

#endif /* _EMC_H_ */

/*! @} */

