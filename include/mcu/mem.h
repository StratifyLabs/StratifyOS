// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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
	void * usage;
} mem_state_t;

typedef struct {
	u16 o_flags; //assigned to MEM_FLAG_IS_RAM or MEM_FLAG_IS_FLASH
	u16 page_count; //number of pages in the section
	u32 address; //beginning of the section
	u32 page_size; //number of bytes in each page
} mem_section_t;

typedef struct {
	u32 usage_size;
	u16 section_count;
	mem_section_t sections[];
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


