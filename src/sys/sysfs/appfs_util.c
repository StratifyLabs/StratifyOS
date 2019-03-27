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


#include <errno.h>
#include <limits.h>
#include "sos/sos.h"

#include "mcu/mcu.h"
#include "mcu/mcu.h"
#include "cortexm/mpu.h"
#include "mcu/mem.h"
#include "cortexm/task.h"
#include "mcu/core.h"
#include "appfs_local.h"
#include "mcu/debug.h"
#include "mcu/wdt.h"

#include "../symbols.h"

#define MCU_LAST_BOOTLOADER_PAGE 3

#define APPFS_REWRITE_MASK 0xFF000000
#define APPFS_REWRITE_RAM_MASK (0x00800000)
#define APPFS_REWRITE_KERNEL_ADDR (0x00FF8000)
#define APPFS_REWRITE_KERNEL_ADDR_MASK (0x7FFF)

static int populate_file_header(const devfs_device_t * dev, appfs_file_t * file, const mem_pageinfo_t * page_info, int type);
static int get_flash_page_type(const devfs_device_t * dev, u32 address, u32 size);
static int is_flash_blank(u32 address, u32 size);
static int read_appfs_file_header(const devfs_device_t * dev, u32 address, appfs_file_t * dest);
static u32 find_protectable_addr(const devfs_device_t * dev, int size, int type, int * page, int skip_protection);
static u32 find_protectable_free(const devfs_device_t * dev, int type, int size, int * page, int skip_protection);


static u8 calc_checksum(const char * name){
	int i;
	u8 checksum;
	checksum = 0;
	for(i=0; i < NAME_MAX-1; i++){
		checksum ^= name[i];
	}
	return checksum;
}

int is_flash_blank(u32 address, u32 size){
	u32 * dest = (u32*)address;
	for(u32 i=0; i < size/sizeof(u32); i++){
		if( *dest++ != 0xffffffff ){
			return 0;
		}
	}
	return 1;
}



int appfs_util_root_erase_pages(const devfs_device_t * dev, int start_page, int end_page){
	int result;
	int i;
	for(i=start_page; i <= end_page; i++){
		mcu_wdt_reset();
		result = dev->driver.ioctl(&(dev->handle), I_MEM_ERASE_PAGE, (void*)i);
		if( result < 0 ){ return result; }
	}
	return 0;
}

int appfs_util_root_get_pageinfo(const devfs_device_t * dev, mem_pageinfo_t * pageinfo){
	return dev->driver.ioctl(&(dev->handle), I_MEM_GETPAGEINFO, pageinfo);
}

int appfs_util_root_get_meminfo(const devfs_device_t * device, mem_info_t * mem_info){
	return device->driver.ioctl(&(device->handle), I_MEM_GETINFO, mem_info);
}

void appfs_util_svcall_get_meminfo(void * args){
	appfs_get_meminfo_t * p = args;
	p->result = appfs_util_root_get_meminfo(p->device, &p->mem_info);
}

void appfs_util_svcall_get_pageinfo(void * args){
	appfs_get_pageinfo_t * p = args;
	p->result = appfs_util_root_get_pageinfo(p->device, &p->page_info);
}

void appfs_util_svcall_erase_pages(void * args){
	appfs_erase_pages_t * p = args;
	p->result = appfs_util_root_erase_pages(p->device, p->start_page, p->end_page);
}


static u32 translate_value(u32 addr, u32 mask, u32 code_start, u32 data_start, u32 total, s32 * loc){
	//check if the value is an address
	u32 ret;
	ret = addr;
	*loc = 0;
	if ( (addr & APPFS_REWRITE_MASK) == mask ){ //matches Text or Data
		ret = addr & ~(APPFS_REWRITE_MASK|APPFS_REWRITE_RAM_MASK);
		if( (addr & APPFS_REWRITE_KERNEL_ADDR) == APPFS_REWRITE_KERNEL_ADDR ){
			//This is a kernel value
			ret = (addr & APPFS_REWRITE_KERNEL_ADDR_MASK)>>2; //convert the address to a table index value
			if( ret < total ){
				//get the symbol location from the symbols table
				if( symbols_table[ret] == 0 ){
					mcu_debug_log_error(MCU_DEBUG_APPFS, "symbol at offset %d (%p) is zero", ret, symbols_table + ret);
					*loc = ret; //this symbol isn't available -- it was removed to save space in the MCU flash
				}
				return symbols_table[ret];
			} else {
				mcu_debug_log_error(MCU_DEBUG_APPFS, "location exceeds total for %p (%d)", addr, total);
				return addr;
				*loc = total;
				return 0;
			}
		} else if (addr & APPFS_REWRITE_RAM_MASK ){
			ret |= data_start;
		} else {
			ret |= code_start;
		}
	}

	return ret;
}

u32 find_protectable_addr(const devfs_device_t * dev, int size, int type, int * page, int skip_protection){
	int i;
	u32 tmp_rbar;
	u32 tmp_rasr;
	mem_pageinfo_t pageinfo;
	int err;
	int mem_type;

	if ( type == MEM_FLAG_IS_FLASH ){
		mem_type = MPU_MEMORY_FLASH;
	} else {
		mem_type = MPU_MEMORY_SRAM;
	}

	i = *page;
	do {
		//go through each page
		pageinfo.num = i;
		pageinfo.o_flags = type;
		if( appfs_util_root_get_pageinfo(dev, &pageinfo) < 0 ){
			return (u32)-1;
		}

		if( skip_protection ){
			err = 0;
		} else {

			//this will return 0 if the address and size is actually protectable
			err = mpu_calc_region(
						TASK_APPLICATION_CODE_MPU_REGION, //doesn't matter what this is
						(void*)pageinfo.addr,
						size,
						MPU_ACCESS_PR_UR,
						mem_type,
						true,
						&tmp_rbar,
						&tmp_rasr
						);
		}

		if ( err == 0 ){
			*page = i;
			return pageinfo.addr;
		}

		i++;
	} while(1);

	return (u32)-1;
}

int check_for_free_space(const devfs_device_t * dev, int start_page, int type, int size){
	mem_pageinfo_t page_info;
	int free_size;
	int last_addr;
	int last_size;
	int ret;

	page_info.o_flags = type;
	page_info.num = start_page;

	free_size = 0;
	last_addr = -1;
	last_size = -1;

	do {

		appfs_util_root_get_pageinfo(dev, &page_info);

		if ( (last_addr != -1) &&  //make sure last_addr is initialized
			  (last_addr + last_size != page_info.addr) ){
			//memory is not contiguous
			if( size <= free_size ){
				return free_size;
			} else {
				return -1;
			}
		}

		last_addr = page_info.addr;
		last_size = page_info.size;
		if ( type == MEM_FLAG_IS_FLASH ){
			//load the file info
			ret = get_flash_page_type(dev, page_info.addr, page_info.size);
		} else {
			//get the type from the ram usage table
			ret = appfs_ram_root_get(dev, page_info.num);
		}

		if ( ret == APPFS_MEMPAGETYPE_FREE ){
			free_size += page_info.size;
		} else {
			if( size <= free_size ){
				return free_size;
			} else {
				return -1;
			}
		}
		page_info.num++;
	} while ( ret >= 0 );

	return free_size;
}

u32 find_protectable_free(const devfs_device_t * dev, int type, int size, int * page, int skip_protection){
	u32 start_addr;
	int tmp;
	int space_available;
	u32 smallest_space_addr;
	int smallest_space_page;

	//find any area for the code
	*page = 0;
	//find an area of memory that is available to write
	smallest_space_addr = (u32)-1;
	space_available = INT_MAX;
	smallest_space_page = 0;

	do {
		start_addr = find_protectable_addr(dev,
													  size,
													  type,
													  page,
													  skip_protection);

		if ( start_addr != (u32)-1 ){
			//could not find any free space

			tmp = check_for_free_space(dev, *page, type, size);
			if( tmp > 0 && tmp < space_available ){
				//there is room here -- find the smallest free space where the program fits
				space_available = tmp;
				smallest_space_addr = start_addr;
				smallest_space_page = *page;
				if( size == space_available ){ //this is a perfect fit
					return smallest_space_addr;
				}

			}

			(*page)++;
		}

	} while( start_addr != (u32)-1 );

	*page = smallest_space_page;

	return smallest_space_addr;
}

/*
static int find_free(const devfs_device_t * dev, int type, int size){
	mem_pageinfo_t pageinfo;
	//find any area for the code

	pageinfo.num = MCU_LAST_BOOTLOADER_PAGE;
	//find an area of flash that is available to write
	do {

		pageinfo.o_flags = type;
		if ( dev->driver.ioctl(&(dev->handle), I_MEM_GET_PAGEINFO, &pageinfo) < 0 ){
			return -1;
		}

		if ( check_for_free_space(dev, pageinfo.num, type, size) == 0 ){
			return pageinfo.addr;
		}

		pageinfo.num++;

	} while(1);

	return -1;
}
 */

const appfs_file_t * appfs_util_getfile(appfs_handle_t * h){
	return (appfs_file_t *)h->type.reg.beg_addr;
}

int appfs_util_root_free_ram(const devfs_device_t * dev, appfs_handle_t * h){
	const appfs_file_t * f;

	if( h->is_install != 0 ){
		return SYSFS_SET_RETURN(EBADF);
	}

	//the RAM info is stored in flash
	f = appfs_util_getfile(h);

	mem_pageinfo_t page_info;
	page_info.addr = f->exec.ram_start;
	page_info.o_flags = MEM_FLAG_IS_QUERY;

	if ( appfs_util_root_get_pageinfo(dev, &page_info) < 0 ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	appfs_ram_root_set(dev, page_info.num, f->exec.ram_size, APPFS_MEMPAGETYPE_FREE);

	return 0;
}

int appfs_util_root_reclaim_ram(const devfs_device_t * dev, appfs_handle_t * h){
	const appfs_file_t * f;
	size_t s;
	size_t page_num;
	mem_pageinfo_t page_info;

	if( h->is_install ){
		return SYSFS_SET_RETURN(EBADF);
	}

	//the RAM info is store in flash
	f = appfs_util_getfile(h);

	page_info.addr = f->exec.ram_start;
	page_info.o_flags = MEM_FLAG_IS_QUERY;

	if ( appfs_util_root_get_pageinfo(dev, &page_info) ){
		return SYSFS_SET_RETURN(EIO);
	}

	page_num = page_info.num;

	for(s=0; s < f->exec.ram_size; s += MCU_RAM_PAGE_SIZE ){
		if( appfs_ram_root_get(dev, page_num++) !=  APPFS_MEMPAGETYPE_FREE ){
			return SYSFS_SET_RETURN(ENOMEM);
		}
	}

	appfs_ram_root_set(dev, page_info.num, f->exec.ram_size, APPFS_MEMPAGETYPE_SYS);

	return 0;
}

static int mem_write_page(const devfs_device_t * dev, appfs_handle_t * h, appfs_installattr_t * attr){
	//now write the buffer
	mem_writepage_t write_page;

	if( (attr->loc + attr->nbyte) > (h->type.install.code_size + h->type.install.data_size) ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	write_page.addr = h->type.install.code_start + attr->loc;
	write_page.nbyte = attr->nbyte;
	memcpy(write_page.buf, attr->buffer, 256);
	return dev->driver.ioctl(&(dev->handle), I_MEM_WRITEPAGE, &write_page);
}

int appfs_util_root_create(const devfs_device_t * dev, appfs_handle_t * h, appfs_installattr_t * attr){
	u32 code_start_addr;
	int type;
	int len;
	int page;
	appfs_file_t * dest;
	dest = (appfs_file_t*)attr->buffer;

	if ( h->is_install == false ){
		return SYSFS_SET_RETURN(EBADF);
	}

	if ( attr->loc == 0 ){

		//This is the header data -- make sure it is complete
		if ( attr->nbyte < sizeof(appfs_file_t) ){
			return SYSFS_SET_RETURN(ENOTSUP);
		}


		if( dest->exec.signature != APPFS_CREATE_SIGNATURE ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		//make sure the name is valid
		len = strnlen(dest->hdr.name, NAME_MAX-2);
		if ( len == (NAME_MAX-2) ){
			//truncate the name if it is too long
			dest->hdr.name[NAME_MAX-1] = 0;
			dest->hdr.name[NAME_MAX-2] = 0;
		}

		//add a checksum to the name
		dest->hdr.name[NAME_MAX-1] = calc_checksum(dest->hdr.name);

		//set the mode to read only
		dest->hdr.mode = 0444;

		//check the options
		dest->exec.o_flags = APPFS_FLAG_IS_FLASH;
		type = MEM_FLAG_IS_FLASH;

		//find space for the code -- this doesn't need to be protectable
		code_start_addr = find_protectable_free(dev, type, dest->exec.code_size, &page, 0);
		if ( code_start_addr == (u32)-1 ){
			return SYSFS_SET_RETURN(ENOSPC);
		}

		//remove the header for read only flash files
		dest->exec.startup = 0;
		dest->exec.code_start = code_start_addr;
		dest->exec.ram_start = 0;
		dest->exec.ram_size = 0;
		dest->exec.data_size = 0;
		dest->exec.o_flags = 0;
		dest->exec.startup = 0;

		h->type.install.code_start = (u32)code_start_addr;
		h->type.install.code_size = dest->exec.code_size;
		h->type.install.data_start = 0;
		h->type.install.data_size = 0;

		h->type.install.rewrite_mask = 0;
		h->type.install.kernel_symbols_total = 0;

	} else {
		if ( (attr->loc & 0x03) ){
			//this is not a word aligned write
			return SYSFS_SET_RETURN(EINVAL);
		}
	}


	//now write the buffer
	return mem_write_page(dev, h, attr);
}



int appfs_util_root_writeinstall(const devfs_device_t * dev, appfs_handle_t * h, appfs_installattr_t * attr){
	union {
		const appfs_file_t * file;
		const u32 * ptr;
	} src;
	int i;
	u32 code_start_addr;
	int code_size;
	u32 data_start_addr;
	int ram_size;
	int type;
	int len;
	int code_page;
	int ram_page;
	s32 loc_err = 0;

	if ( h->is_install == false ){
		return SYSFS_SET_RETURN(EBADF);
	}

	union {
		appfs_file_t file;
		u32 buf[attr->nbyte>>2];
	} dest;

	src.ptr = (const u32*)attr->buffer;

	if ( attr->loc == 0 ){
		//This is the header data -- make sure it is complete
		if ( attr->nbyte < sizeof(appfs_file_t) ){
			mcu_debug_log_error(MCU_DEBUG_APPFS, "Page size is less than min");
			return SYSFS_SET_RETURN(ENOTSUP);
		}

		//make sure the name is valid
		memset(&dest.file, 0, sizeof(appfs_file_t));
		memcpy(&dest.file, src.file, attr->nbyte);
		len = strnlen(dest.file.hdr.name, NAME_MAX-2);
		if ( len == (NAME_MAX-2) ){
			//truncate the name if it is too long
			dest.file.hdr.name[NAME_MAX-2] = 0;
		}

		len = strnlen(dest.file.hdr.id, NAME_MAX-2);
		if ( len == (NAME_MAX-2) ){
			//truncate the name if it is too long
			dest.file.hdr.id[NAME_MAX-2] = 0;
		}

		//add a checksum to the name
		dest.file.hdr.name[NAME_MAX-1] = calc_checksum(dest.file.hdr.name);
		dest.file.hdr.id[NAME_MAX-1] = calc_checksum(dest.file.hdr.id);

		//set mode to read/exec
		dest.file.hdr.mode = 0555;

		//is signature correct
		if ( src.file->exec.signature != symbols_table[0] ){
			mcu_debug_log_error(MCU_DEBUG_APPFS, "Not executable");
			return SYSFS_SET_RETURN(ENOEXEC);
		}

		//check the options
		if ( (src.file->exec.o_flags) & APPFS_FLAG_IS_FLASH ){
			//This should be installed in flash
			type = MEM_FLAG_IS_FLASH;
		} else {
			//This should be install in RAM
			type = MEM_FLAG_IS_RAM;
		}

		code_size = src.file->exec.code_size + src.file->exec.data_size; //code plus initialized data
		ram_size = src.file->exec.ram_size;

		//find space for the code
		code_start_addr = find_protectable_free(dev, type, code_size, &code_page, 0);
		if ( code_start_addr == (u32)-1 ){
			mcu_debug_log_error(MCU_DEBUG_APPFS, "No exec region available");
			return SYSFS_SET_RETURN(ENOSPC);
		}
		mcu_debug_log_info(MCU_DEBUG_APPFS, "Code start addr is %p", code_start_addr);

		if ( !((src.file->exec.o_flags) & APPFS_FLAG_IS_FLASH) ){ //for RAM app's mark the RAM usage
			//mark the range as SYS
			appfs_ram_root_set(dev,
									 code_page,
									 code_size,
									 APPFS_MEMPAGETYPE_SYS);

			//mark the first page as USER
			appfs_ram_root_set(dev,
									 code_page,
									 MCU_RAM_PAGE_SIZE, //mark the first page as USER
									 APPFS_MEMPAGETYPE_USER);
		}

		ram_page = 0;
		data_start_addr = find_protectable_free(dev, MEM_FLAG_IS_RAM, ram_size, &ram_page, 0);
		if( data_start_addr == (u32)-1 ){
			if ( !((src.file->exec.o_flags) & APPFS_FLAG_IS_FLASH) ){ //for RAM app's mark the RAM usage
				//free the code section
				appfs_ram_root_set(dev,
										 code_page,
										 code_size,
										 APPFS_MEMPAGETYPE_FREE);

			}
			mcu_debug_log_error(MCU_DEBUG_APPFS, "No RAM region available %d", ram_size);
			return SYSFS_SET_RETURN(ENOSPC);
		}
		mcu_debug_log_info(MCU_DEBUG_APPFS, "Data start addr is %p", data_start_addr);

		h->type.install.code_start = (u32)code_start_addr;
		h->type.install.code_size = code_size;
		h->type.install.data_start = (u32)data_start_addr;
		h->type.install.data_size = src.file->exec.data_size;

		h->type.install.rewrite_mask = (u32)(src.file->exec.code_start) & APPFS_REWRITE_MASK;
		h->type.install.kernel_symbols_total = symbols_total();

		appfs_ram_root_set(dev,
								 ram_page,
								 ram_size,
								 APPFS_MEMPAGETYPE_SYS);

		dest.file.exec.code_start = code_start_addr;
		dest.file.exec.ram_start = data_start_addr;
		dest.file.exec.startup = translate_value((u32)dest.file.exec.startup,
															  h->type.install.rewrite_mask,
															  h->type.install.code_start,
															  h->type.install.data_start,
															  h->type.install.kernel_symbols_total,
															  &loc_err);

		for(i=sizeof(appfs_file_t) >> 2; i < attr->nbyte >> 2; i++){
			dest.buf[i] = translate_value(src.ptr[i],
													h->type.install.rewrite_mask,
													h->type.install.code_start,
													h->type.install.data_start,
													h->type.install.kernel_symbols_total,
													&loc_err);
			if( loc_err != 0 ){
				mcu_debug_log_error(MCU_DEBUG_APPFS, "Code relocation error: %d", loc_err);
				return SYSFS_SET_RETURN_WITH_VALUE(EIO, loc_err);
			}
		}

	} else {

		if ( (attr->loc & 0x03) ){
			//this is not a word aligned write
			mcu_debug_log_error(MCU_DEBUG_APPFS, "word alignment error 0x%X\n", attr->loc);
			return SYSFS_SET_RETURN(EINVAL);
		}
		for(i=0; i < attr->nbyte >> 2; i++){
			dest.buf[i] = translate_value(src.ptr[i],
													h->type.install.rewrite_mask,
													h->type.install.code_start,
													h->type.install.data_start,
													h->type.install.kernel_symbols_total,
													&loc_err);
			if( loc_err != 0 ){
				mcu_debug_log_error(MCU_DEBUG_APPFS, "Code relocation error %d", loc_err);
				return SYSFS_SET_RETURN_WITH_VALUE(EIO, loc_err);
			}
		}
	}

	memcpy(attr->buffer, &dest, attr->nbyte);

	//now write buffer
	return mem_write_page(dev, h, attr);
}

int get_flash_page_type(const devfs_device_t * dev, u32 address, u32 size){
	int len;
	appfs_file_t appfs_file;

	if( is_flash_blank(address, size) ){
		return APPFS_MEMPAGETYPE_FREE;
	}

	read_appfs_file_header(dev, address, &appfs_file);
	len = strnlen(appfs_file.hdr.name, NAME_MAX-2);
	if ( (len == NAME_MAX - 2) || //check if the name is short enough
		  (len != strspn(appfs_file.hdr.name, sysfs_validset)) || //check if only valid characters are present
		  (appfs_file.hdr.name[NAME_MAX-1] != calc_checksum(appfs_file.hdr.name)) || //check for a valid checksum
		  (len == 0)
		  ){

		return APPFS_MEMPAGETYPE_SYS;

	}

	return APPFS_MEMPAGETYPE_USER;
}

int appfs_util_is_executable(const appfs_file_t * info){
	// do a priv read of the signature
	if ( info->exec.signature != symbols_table[0] ){
		return 0;
	}
	return 1;
}

int populate_file_header(const devfs_device_t * device, appfs_file_t * file, const mem_pageinfo_t * page_info, int memory_type){
	char hex_num[9];
	int page_type;

	if ( memory_type == MEM_FLAG_IS_FLASH ){
		page_type = get_flash_page_type(device, page_info->addr, page_info->size);
	} else {
		page_type = appfs_ram_root_get(device, page_info->num);
	}

	htoa(hex_num, page_info->num);
	switch(page_type){
		case APPFS_MEMPAGETYPE_SYS:
			strcpy(file->hdr.name, ".sys");
			strcat(file->hdr.name, hex_num);
			file->hdr.mode = S_IFREG;
			memset(&(file->exec), 0, sizeof(appfs_exec_t));
			break;
		case APPFS_MEMPAGETYPE_FREE:
			strcpy(file->hdr.name, ".free");
			strcat(file->hdr.name, hex_num);
			file->hdr.mode = S_IFREG;
			memset(&(file->exec), 0, sizeof(appfs_exec_t));
			break;
		case APPFS_MEMPAGETYPE_USER:
			break;
	}

	return page_type;
}

void appfs_util_svcall_get_fileinfo(void * args){
	appfs_get_fileinfo_t * p = args;
	p->result = appfs_util_root_get_fileinfo(
				p->device,
				&p->file_info,
				p->page,
				p->type,
				&p->size);
}

int read_appfs_file_header(const devfs_device_t * dev, u32 address, appfs_file_t * dest){
	//now that addr is available -- read the address
	devfs_async_t async;

	async.buf = dest;
	async.nbyte = sizeof(appfs_file_t);
	async.handler.context = NULL;
	async.loc = (int)address;
	async.tid = task_get_current();

	//read the memory directly to get the file header
	return dev->driver.read(&dev->handle, &async);
}


int appfs_util_root_get_fileinfo(const devfs_device_t * dev, appfs_file_t * file_info, int page, int type, int * size){
	int file_type;
	int result;
	mem_pageinfo_t page_info;
	page_info.num = page;
	page_info.o_flags = type;

	//this will get the size and address of the page
	result = appfs_util_root_get_pageinfo(dev, &page_info);
	if( result < 0 ){ return result; }

	result = read_appfs_file_header(dev, page_info.addr, file_info);
	if( result < 0 ){ return result;	}

	//get the header info for free and sys files
	file_type = populate_file_header(dev, file_info, &page_info, type);

	if ( size != NULL ){
		if ( file_type == APPFS_MEMPAGETYPE_USER ){
			*size = file_info->exec.code_size + file_info->exec.data_size;
		} else {
			*size = page_info.size;
		}
	}

	return file_type;
}

int appfs_util_lookupname(const devfs_device_t * device,
								  const char * path,
								  appfs_file_t * file_info,
								  mem_pageinfo_t * page_info,
								  int type,
								  int * size){
	appfs_get_fileinfo_t get_fileinfo_args;
	appfs_get_pageinfo_t get_pageinfo_args;

	get_fileinfo_args.device = device;
	get_fileinfo_args.type = type;

	get_pageinfo_args.device = device;
	get_pageinfo_args.page_info.o_flags = type;

	if ( strnlen(path, NAME_MAX-2) == NAME_MAX-2 ){
		return -1;
	}

	get_fileinfo_args.page = 0;
	do {
		//go through each page

		cortexm_svcall(appfs_util_svcall_get_fileinfo, &get_fileinfo_args);
		//if ( appfs_util_root_get_fileinfo(device, file_info, i, type, size) < 0){
		//	return -1;
		//}

		if( get_fileinfo_args.result < 0 ){ return get_fileinfo_args.result; }

		if( size ){
			*size = get_fileinfo_args.size;
		}
		get_pageinfo_args.page_info.num = get_fileinfo_args.page;
		cortexm_svcall(appfs_util_svcall_get_pageinfo, &get_pageinfo_args);
		if( get_pageinfo_args.result < 0 ){ return get_fileinfo_args.result; }

		if ( strncmp(path, get_fileinfo_args.file_info.hdr.name, NAME_MAX) == 0 ){
			memcpy(file_info, &get_fileinfo_args.file_info, sizeof(appfs_file_t));
			memcpy(page_info, &get_pageinfo_args.page_info, sizeof(mem_pageinfo_t));
			return 0;
		}

		get_fileinfo_args.page++;

	} while(1);

	//name not found
	return -1;
}





