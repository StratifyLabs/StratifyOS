/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup FLASH_DEV Flash
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_FLASH_H_
#define _MCU_FLASH_H_


#include "sos/dev/flash.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \details
 *
 * The following is an example of how to open the EINT:
 * \code
 * flash_attr_t flash_cfg;
 * if ( flash_open(0, &flash_cfg) ){
 *  //Failed
 * }
 * \endcode
 *
 * \sa periph_open()
 *
 */
int mcu_flash_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_flash_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_flash_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_flash_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_flash_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;


int mcu_flash_getinfo(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_setaction(int port, void * ctl) MCU_PRIV_CODE;

int mcu_flash_eraseaddr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_erasepage(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_getpage(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_getsize(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_getpageinfo(int port, void * ctl) MCU_PRIV_CODE;
int mcu_flash_writepage(int port, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif // _MCU_FLASH_H_

/*! @} */


