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

#ifndef APPFS_LOCAL_H_
#define APPFS_LOCAL_H_

#include "config.h"
#include <stdbool.h>
#include "sos/dev/appfs.h"
#include "mcu/debug.h"
#include "mcu/mem.h"
#include "sos/fs/sysfs.h"


typedef struct {
	u32 code_start /*! the new value */;
	u32 code_size;
	u32 data_start /*! the new data start */;
	u32 data_size;
	u32 rewrite_mask;
	u32 kernel_symbols_total;
} appfs_util_handle_t;

#define APPFS_MEMPAGETYPE_FREE 0
#define APPFS_MEMPAGETYPE_SYS 1
#define APPFS_MEMPAGETYPE_USER 2
#define APPFS_MEMPAGETYPE_MASK 0x03
#define APPFS_MEMPAGETYPE_INVALID (-1)

typedef struct {
	u32 beg_addr /*! the address of the beginning of the file */;
	u32 size /*! the current size of the file */;
	int page /*! the start page of the open file */;
	u32 o_flags /*! Memory flags (MEM_FLAG_IS_FLASH or MEM_FLAG_IS_RAM) */;
	u32 mode /*! \brief File mode 0444 for data and 0555 for executables */;
} appfs_reg_handle_t;


typedef struct {
	u8 is_install /*! boolean for the .install file */;
	union {
		appfs_util_handle_t install;
		appfs_reg_handle_t reg;
	} type;
} appfs_handle_t;

typedef struct {
	const devfs_device_t * device;
	u32 page;
	u32 size;
	int type;
} appfs_ram_t;

typedef struct {
	const devfs_device_t * device;
	mem_pageinfo_t page_info;
	int result;
} appfs_get_pageinfo_t;

typedef struct {
	const devfs_device_t * device;
	appfs_file_t file_info;
	int page;
	int type;
	int size;
	int result;
} appfs_get_fileinfo_t;

typedef struct {
	const devfs_device_t * device;
	mem_info_t mem_info;
	int result;
} appfs_get_meminfo_t;

typedef struct {
	const devfs_device_t * device;
	u32 start_page;
	u32 end_page;
	int result;
} appfs_erase_pages_t;


//file utilities
int appfs_util_lookupname(const devfs_device_t * device,
								  const char * path,
								  appfs_file_t * file_info,
								  mem_pageinfo_t * page_info,
								  int type,
								  int * size);
const appfs_file_t * appfs_util_getfile(appfs_handle_t * h);
int appfs_util_is_executable(const appfs_file_t * info);

//call through cortexm_svcall()
void appfs_util_svcall_get_fileinfo(void * args) MCU_ROOT_EXEC_CODE;
void appfs_util_svcall_get_pageinfo(void * args) MCU_ROOT_EXEC_CODE;
void appfs_util_svcall_get_meminfo(void * args) MCU_ROOT_EXEC_CODE;
void appfs_util_svcall_erase_pages(void * args) MCU_ROOT_EXEC_CODE;
void appfs_ram_svcall_get(void * args) MCU_ROOT_EXEC_CODE;
void appfs_ram_svcall_set(void * args) MCU_ROOT_EXEC_CODE;


//call in root mode only
int appfs_util_root_writeinstall(const devfs_device_t * device, appfs_handle_t * h, appfs_installattr_t * attr) MCU_ROOT_CODE;
int appfs_util_root_create(const devfs_device_t * device, appfs_handle_t * h, appfs_installattr_t * attr) MCU_ROOT_CODE;
int appfs_util_root_free_ram(const devfs_device_t * device, appfs_handle_t * h) MCU_ROOT_CODE;
int appfs_util_root_reclaim_ram(const devfs_device_t * device, appfs_handle_t * h) MCU_ROOT_CODE;
int appfs_util_root_get_meminfo(const devfs_device_t * device, mem_info_t * mem_info) MCU_ROOT_CODE;
int appfs_util_root_get_pageinfo(const devfs_device_t * device, mem_pageinfo_t * pageinfo) MCU_ROOT_CODE;
int appfs_util_root_get_fileinfo(const devfs_device_t * device, appfs_file_t * file_info, int page, int type, int * size) MCU_ROOT_CODE;
int appfs_util_root_erase_pages(const devfs_device_t * device, int start_page, int end_page) MCU_ROOT_CODE;
void appfs_ram_root_init(const devfs_device_t * device) MCU_ROOT_CODE;
int appfs_ram_root_get(const devfs_device_t * device, u32 page) MCU_ROOT_CODE;
void appfs_ram_root_set(const devfs_device_t * device, u32 page, u32 size, int type) MCU_ROOT_CODE;


#endif /* APPFS_LOCAL_H_ */
