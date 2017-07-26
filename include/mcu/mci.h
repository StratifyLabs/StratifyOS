/* Copyright 2011 MCU_PRIV_CODE; 
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


/*! \details
 * \sa periph_open()
 *
 */
int mcu_mci_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_mci_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_mci_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_mci_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_mci_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;

int mcu_mci_getinfo(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_mci_setattr(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;
int mcu_mci_setaction(const devfs_handle_t * handle, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif /* mcu_mci_H_ */

/*! @} */
