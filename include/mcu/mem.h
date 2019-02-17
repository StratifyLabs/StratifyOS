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

/*! \addtogroup MEM_DEV Memory
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_MEM_H_
#define _MCU_MEM_H_

#include "sos/dev/mem.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} mem_state_t;


typedef struct {

} mem_config_t;



int mcu_mem_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
int mcu_mem_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_mem_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_mem_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int mcu_mem_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_mem_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_mem_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_mem_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

int mcu_mem_erasepage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_mem_getpageinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_mem_writepage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

//Non ioctl call - used directly by Stratify OS appfs
int mcu_mem_getsyspage();


#ifdef __cplusplus
}
#endif


#endif // _MCU_MEM_H_

/*! @} */


