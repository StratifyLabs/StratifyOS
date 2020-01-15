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

#include "mcu/arch.h"
#include "cortexm/cortexm.h"
#include "mcu/appfs.h"
#include "mcu/flash.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/wdt.h"
#include "mcu/bootloader.h"

#define DECLARE_APPFS_CONFIG() const appfs_mem_config_t * config = handle->config
#define DECLARE_APPFS_CONFIG_STATE() const appfs_mem_config_t * config = handle->config; appfs_mem_state_t * state = handle->state

#define INVALID_PAGE ((u32)-1)
static const appfs_mem_section_t * get_page_section_info(const appfs_mem_config_t * config, u32 page, u32 type, u32 * offset);

static u32 get_memory_page_count(const appfs_mem_config_t * config, u32 type);
static u32 get_memory_size(const appfs_mem_config_t * config, u32 type);

static int get_page_info(const appfs_mem_config_t * config, u32 page, u32 type, mem_pageinfo_t * info);
static u32 get_page(const appfs_mem_config_t * config, u32 address, u32 size, u32 * type);

int appfs_mem_open(const devfs_handle_t * handle){
	DECLARE_APPFS_CONFIG();
	if( config->flash_driver ){
		return config->flash_driver->driver.open( &config->flash_driver->handle );
	}
	return 0;
}

int appfs_mem_read(const devfs_handle_t * handle, devfs_async_t * async){
	DECLARE_APPFS_CONFIG();
	//if RAM, write directly
	u32 type;
	get_page(config, (u32)async->loc, (u32)async->nbyte, &type);

	if( type == 0 ){ return SYSFS_SET_RETURN(EINVAL); }

	memcpy(async->buf, (void*)async->loc, async->nbyte);

	//if flash, write with driver
	return async->nbyte;
}

int appfs_mem_write(const devfs_handle_t * handle, devfs_async_t * async){
	DECLARE_APPFS_CONFIG();
	//if RAM, write directly
	u32 type;
	get_page(config, (u32)async->loc, (u32)async->nbyte, &type);

	if( type == 0 ){ return SYSFS_SET_RETURN(EINVAL); }

	if( type & MEM_FLAG_IS_RAM ){
		//check to see if the memcpy fits in RAM
		memcpy((void*)async->loc, async->buf, async->nbyte);
	} else {
		return SYSFS_SET_RETURN(ENOTSUP);
	}

	//if flash, write with driver
	return async->nbyte;
}

int appfs_mem_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	switch(request){
		case I_MEM_GETVERSION: return MEM_VERSION;
		case I_MEM_GETINFO: return appfs_mem_getinfo(handle, ctl);
		case I_MEM_SETATTR: return appfs_mem_setattr(handle, ctl);
		case I_MEM_SETACTION: return appfs_mem_setaction(handle, ctl);
		case I_MEM_ERASEPAGE: return appfs_mem_erasepage(handle, ctl);
		case I_MEM_GETPAGEINFO: return appfs_mem_getpageinfo(handle, ctl);
		case I_MEM_WRITEPAGE: return appfs_mem_writepage(handle, ctl);
	}
	return SYSFS_SET_RETURN(EINVAL);
}

int appfs_mem_close(const devfs_handle_t * handle){
	MCU_UNUSED_ARGUMENT(handle);
	return 0;
}

int appfs_mem_getinfo(const devfs_handle_t * handle, void * ctl){
	DECLARE_APPFS_CONFIG();
	mem_info_t * info = ctl;
	if( ctl == 0 ){ return SYSFS_SET_RETURN(EINVAL); }
	info->flash_pages = get_memory_page_count(config, MEM_FLAG_IS_FLASH);
	info->flash_size = get_memory_size(config, MEM_FLAG_IS_FLASH);
	info->ram_pages = get_memory_page_count(config, MEM_FLAG_IS_RAM);
	info->ram_size = get_memory_size(config, MEM_FLAG_IS_RAM);
	info->external_ram_pages = get_memory_page_count(config, MEM_FLAG_IS_RAM | MEM_FLAG_IS_EXTERNAL);
	info->external_ram_size = get_memory_size(config, MEM_FLAG_IS_RAM | MEM_FLAG_IS_EXTERNAL);
	info->tightlycoupled_ram_pages = get_memory_page_count(config, MEM_FLAG_IS_RAM | MEM_FLAG_IS_TIGHTLY_COUPLED);
	info->tightlycoupled_ram_size = get_memory_size(config, MEM_FLAG_IS_RAM | MEM_FLAG_IS_TIGHTLY_COUPLED);
	info->system_ram_page = config->system_ram_page;
	info->usage = config->usage;
	info->usage_size = config->usage_size;
	return 0;
}

int appfs_mem_setattr(const devfs_handle_t * handle, void * ctl){
	MCU_UNUSED_ARGUMENT(handle);
	MCU_UNUSED_ARGUMENT(ctl);
	return 0;
}

int appfs_mem_setaction(const devfs_handle_t * handle, void * ctl){
	MCU_UNUSED_ARGUMENT(handle);
	MCU_UNUSED_ARGUMENT(ctl);
	return SYSFS_SET_RETURN(ENOTSUP);
}


//erases a flash page
int appfs_mem_erasepage(const devfs_handle_t * handle, void * ctl){
	DECLARE_APPFS_CONFIG();
	if( config->flash_driver != 0 ){
		return config->flash_driver->driver.ioctl(&config->flash_driver->handle, I_FLASH_ERASEPAGE, ctl);
	}
	return SYSFS_SET_RETURN(ENOTSUP);
}

int appfs_mem_getpageinfo(const devfs_handle_t * handle, void * ctl){
	DECLARE_APPFS_CONFIG();
	mem_pageinfo_t * pageinfo = ctl;

	if( pageinfo->o_flags & MEM_FLAG_IS_QUERY ){
		u32 type;
		pageinfo->num = get_page(config, pageinfo->addr, 0, &type);
		pageinfo->o_flags = type;
		//pageinfo->size = get_page_size(config, pageinfo->num, type & (MEM_FLAG_IS_RAM|MEM_FLAG_IS_FLASH));
		if( type == 0 ){ return SYSFS_SET_RETURN(EINVAL); }
		//return 0;
	}

	if( get_page_info(
			 config,
			 pageinfo->num,
			 pageinfo->o_flags,
			 pageinfo
			 ) < 0 ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	return 0;

#if 0
	size = get_page_size(config, pageinfo->num, pageinfo->o_flags);
	mcu_debug_printf("get info for page %d 0x%lX %ld\n", pageinfo->num, pageinfo->o_flags, size);
	if (size == 0 ){ return SYSFS_SET_RETURN(EINVAL); }
	pageinfo->addr = get_page_addr(config, pageinfo->num, pageinfo->o_flags);
	pageinfo->size = size;
	return 0;
#endif
}

int appfs_mem_writepage(const devfs_handle_t * handle, void * ctl){
	DECLARE_APPFS_CONFIG();
	mem_writepage_t * write_page_info = ctl;
	u32 type;
	int result;

	result = get_page(config, (u32)write_page_info->addr, (u32)write_page_info->nbyte, &type);

	if( type == 0 ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	if( type & MEM_FLAG_IS_RAM ){
		//check to see if the memcpy fits in RAM
		mem_writepage_t * write_page_info = ctl;
		memcpy((void*)write_page_info->addr, write_page_info->buf, write_page_info->nbyte);
		result = write_page_info->nbyte;
	} else {
		if( config->flash_driver != 0 ){
			result = config->flash_driver->driver.ioctl(&config->flash_driver->handle, I_FLASH_WRITEPAGE, ctl);
		} else {
			return SYSFS_SET_RETURN(ENOTSUP);
		}
	}

	return result;
}

const appfs_mem_section_t * get_page_section_info(
		const appfs_mem_config_t * config,
		u32 page,
		u32 type,
		u32 * offset){
	u32 find_page = 0;
	for(u32 i=0; i < config->section_count; i++){
		if( config->sections[i].o_flags & type ){ //does page match any flag in type -- ram types will have same numbering
			if( page < find_page + config->sections[i].page_count ){
				//page lands on this page
				*offset = page - find_page;
				return config->sections + i;
			}
			find_page += config->sections[i].page_count;
		}
	}
	return 0;
}

u32 get_page(const appfs_mem_config_t * config, u32 address, u32 size, u32 * type){
	u32 ram_page = 0;
	u32 flash_page = 0;
	*type = 0; //zero means it didn't find anything
	for(u32 i=0; i < config->section_count; i++){
		//is the address in this section?
		if( (address >= config->sections[i].address) &&
			 ((address + size) <= config->sections[i].address + config->sections[i].page_size * config->sections[i].page_count)
			 ){
			//what is the page offset?
			u32 offset = (address - config->sections[i].address) / config->sections[i].page_size;
			*type = config->sections[i].o_flags;
			u32 page;
			if( config->sections[i].o_flags & MEM_FLAG_IS_FLASH ){
				page = flash_page;
			} else {
				page = ram_page;
			}
			return page + offset;
		}

		if( config->sections[i].o_flags & MEM_FLAG_IS_FLASH ){
			flash_page += config->sections[i].page_count;
		} else {
			ram_page += config->sections[i].page_count;
		}
	}
	return INVALID_PAGE;
}

u32 get_memory_page_count(const appfs_mem_config_t * config, u32 type){
	u32 pages = 0;
	for(u32 i=0; i < config->section_count; i++){
		if( config->sections[i].o_flags & type ){
			pages += config->sections[i].page_count;
		}
	}
	return pages;
}

u32 get_memory_size(const appfs_mem_config_t * config, u32 type){
	u32 size = 0;
	for(u32 i=0; i < config->section_count; i++){
		if( config->sections[i].o_flags & type ){
			size += (config->sections[i].page_count * config->sections[i].page_size);
		}
	}
	return size;
}

int get_page_info(
		const appfs_mem_config_t * config,
		u32 page,
		u32 type,
		mem_pageinfo_t * info
		){
	u32 offset;
	const appfs_mem_section_t * section =
			get_page_section_info(
				config,
				page,
				type,
				&offset
				);

	if( section == 0 ){ return -1; }
	info->num = page;
	info->addr = section->address + offset * section->page_size;
	info->o_flags = section->o_flags;
	info->size = section->page_size;
	return 0;
}


