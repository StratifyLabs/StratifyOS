/* Copyright 2011-2019 Tyler Gilbert;
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


#ifndef _MCU_APPFS_H_
#define _MCU_APPFS_H_

#include "sos/dev/mem.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {

} appfs_mem_state_t;

typedef struct {
	u16 o_flags; //assigned to MEM_FLAG_IS_RAM or MEM_FLAG_IS_FLASH
	u16 page_count; //number of pages in the section
	u32 address; //beginning of the section
	u32 page_size; //number of bytes in each page
} appfs_mem_section_t;

typedef struct {
	u16 usage_size;
	u16 section_count;
	u32 * usage;
	u32 system_ram_page;
	const devfs_device_t * flash_driver;
	const appfs_mem_section_t sections[];
} appfs_mem_config_t;

int appfs_mem_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
int appfs_mem_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int appfs_mem_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int appfs_mem_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int appfs_mem_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int appfs_mem_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int appfs_mem_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int appfs_mem_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

int appfs_mem_erasepage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int appfs_mem_getpageinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int appfs_mem_writepage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;


#ifdef __cplusplus
}
#endif


#endif // _MCU_APPFS_H_



