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

/*! \addtogroup MEM_DEV Memory
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_MEM_H_
#define _MCU_MEM_H_


#include "../iface/dev/mem.h"
#include "iface/device_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \details
 *
 * The following is an example of how to open the EINT:
 * \code
 * mem_attr_t mem_cfg;
 * if ( mem_open(0, &mem_cfg) ){
 *  //Failed
 * }
 * \endcode
 *
 * \sa periph_open()
 *
 */
int mcu_mem_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
/*! \details
 * \sa periph_read()
 *
 */
int mcu_mem_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_write()
 */
int mcu_mem_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_mem_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_mem_close(const device_cfg_t * cfg) MCU_PRIV_CODE;


int mcu_mem_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_setaction(int port, void * ctl) MCU_PRIV_CODE;

int mcu_mem_eraseaddr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_erasepage(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_getpage(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_getsize(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_getpageinfo(int port, void * ctl) MCU_PRIV_CODE;
int mcu_mem_writepage(int port, void * ctl) MCU_PRIV_CODE;

int _mcu_mem_getsyspage() MCU_PRIV_CODE;


#ifdef __cplusplus
}
#endif


#endif // _MCU_MEM_H_

/*! @} */


