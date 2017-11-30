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

static void appfs_util_privloadfileinfo(void * args) MCU_PRIV_EXEC_CODE;
static int get_hdrinfo(appfs_file_t * file, int page, int type);
static int get_filesize(const devfs_device_t * dev, priv_load_fileinfo_t * args, int filetype);

static void priv_op_erase_pages(void * args) MCU_PRIV_EXEC_CODE;

static u8 calc_checksum(const char * name){
	int i;
	u8 checksum;
	checksum = 0;
	for(i=0; i < NAME_MAX-1; i++){
		checksum ^= name[i];
	}
	return checksum;
}


typedef struct {
	int ret;
	const devfs_device_t * dev;
	devfs_async_t op;
	int start_page;
	int end_page;
} priv_op_t;

void priv_op_erase_pages(void * args){
	int i;
	priv_op_t * p = args;
	for(i=p->start_page; i <= p->end_page; i++){
		mcu_wdt_reset();
		p->ret = p->dev->driver.ioctl(&(p->dev->handle), I_MEM_ERASE_PAGE, (void*)i);
	}
}

int appfs_util_erasepages(const devfs_device_t * dev, int start_page, int end_page){
	priv_op_t args;
	args.dev = dev;
	args.start_page = start_page;
	args.end_page = end_page;
	cortexm_svcall(priv_op_erase_pages, &args);
	return 0;
}

int appfs_util_getpageinfo(const devfs_device_t * dev, mem_pageinfo_t * pageinfo){
	return dev->driver.ioctl(&(dev->handle), I_MEM_GETPAGEINFO, pageinfo);
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
					*loc = ret; //this symbol isn't available -- it was removed to save space in the MCU flash
				}
				return symbols_table[ret];
			} else {
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

static int find_protectable_addr(const devfs_device_t * dev, int size, int type, int * page){
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
		if ( dev->driver.ioctl(&(dev->handle), I_MEM_GETPAGEINFO, &pageinfo) < 0 ){
			return -1;
		}

		err = mpu_calc_region(
				6,
				(void*)pageinfo.addr,
				size,
				MPU_ACCESS_PR_UR,
				mem_type,
				true,
				&tmp_rbar,
				&tmp_rasr
		);

		if ( err == 0 ){
			*page = i;
			return pageinfo.addr;
		}


		i++;
	} while(1);

	return -1;
}

int check_for_free_space(const devfs_device_t * dev, int start_page, int type, int size){
	priv_load_fileinfo_t info;
	int free_size;
	int last_addr;
	int last_size;
	int ret;

	info.dev = dev;
	info.pageinfo.o_flags = type;

	info.pageinfo.num = start_page;
	free_size = 0;
	last_addr = -1;
	last_size = -1;

	do {
		if ( type == MEM_FLAG_IS_FLASH ){
			appfs_util_privloadfileinfo(&info);
		} else {
			appfs_util_getpageinfo(dev, &info.pageinfo);
		}

		if ( (last_addr != -1) &&  //make sure last_addr is initialized
				(last_addr + last_size != info.pageinfo.addr) ){
			//memory is not contiguous
			if( size <= free_size ){
				return free_size;
			} else {
				return -1;
			}
		}

		last_addr = info.pageinfo.addr;
		last_size = info.pageinfo.size;
		if ( type == MEM_FLAG_IS_FLASH ){
			ret = appfs_util_getflashpagetype(&info.fileinfo.hdr);
		} else {
			ret = appfs_ram_getusage(info.pageinfo.num);
		}

		if ( ret == APPFS_MEMPAGETYPE_FREE ){
			free_size += info.pageinfo.size;
		} else {
			if( size <= free_size ){
				return free_size;
			} else {
				return -1;
			}
		}
		info.pageinfo.num++;
	} while ( ret >= 0 );

	return free_size;
}

static int find_protectable_free(const devfs_device_t * dev, int type, int size, int * page){
	int start_addr;
	int tmp;
	int space_available;
	int smallest_space_addr;
	int smallest_space_page;

	//find any area for the code
	*page = 0;
	//find an area of memory that is available to write
	smallest_space_addr = -1;
	space_available = INT_MAX;
	smallest_space_page = 0;

	do {
		start_addr = find_protectable_addr(dev,
				size,
				type,
				page);
		if ( start_addr < 0 ){
			//could not find any free space
			break;
		}


		tmp = check_for_free_space(dev, *page, type, size);
		if( tmp > 0 ){
			//there is room here -- find the smallest free space where the program fits
			if( tmp < space_available ){
				space_available = tmp;
				smallest_space_addr = start_addr;
				smallest_space_page = *page;
				if( size == space_available ){ //this is as good as it gets
					return smallest_space_addr;
				}
			}
		}

		(*page)++;

	} while(1);

	if( smallest_space_addr == -1 ){
		errno = ENOSPC;
	}

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

int appfs_util_priv_free_ram(const devfs_device_t * dev, appfs_handle_t * h){
	priv_load_fileinfo_t args;
	const appfs_file_t * f;

	if( h->is_install != 0 ){
		errno = EBADF;
		return -1;
	}

	//the RAM info is stored in flash
	f = appfs_util_getfile(h);

	args.pageinfo.addr = f->exec.ram_start;
	args.pageinfo.o_flags = MEM_FLAG_IS_QUERY;

	if ( appfs_util_getpageinfo(dev, &args.pageinfo) < 0 ){
		errno = EINVAL;
		return -1;
	}

	if ( appfs_priv_ram_setusage(args.pageinfo.num, f->exec.ram_size, APPFS_MEMPAGETYPE_FREE) < 0 ){
		return -1;
	}

	return 0;
}

int appfs_util_priv_reclaim_ram(const devfs_device_t * dev, appfs_handle_t * h){
	priv_load_fileinfo_t args;
	const appfs_file_t * f;
	size_t s;
	size_t page_num;

	if( h->is_install ){
		errno = EBADF;
		return -1;
	}

	//the RAM info is store in flash
	f = appfs_util_getfile(h);

	args.pageinfo.addr = f->exec.ram_start;
	args.pageinfo.o_flags = MEM_FLAG_IS_QUERY;

	if ( appfs_util_getpageinfo(dev, &args.pageinfo) ){
		return -1;
	}

	page_num = args.pageinfo.num;

	for(s=0; s < f->exec.ram_size; s += DEVICE_RAM_PAGE_SIZE ){
		if( appfs_ram_getusage(page_num++) !=  APPFS_MEMPAGETYPE_FREE ){
			errno = ENOMEM;
			return -1;
		}
	}

	if ( appfs_priv_ram_setusage(args.pageinfo.num, f->exec.ram_size, APPFS_MEMPAGETYPE_SYS) < 0 ){
		return -1;
	}

	return 0;
}

static int mem_write_page(const devfs_device_t * dev, appfs_handle_t * h, appfs_installattr_t * attr){
	//now write the buffer
	mem_writepage_t write_page;

	if( (attr->loc + attr->nbyte) > (h->type.install.code_size + h->type.install.data_size) ){
		errno = EINVAL;
		return -1;
	}

	write_page.addr = h->type.install.code_start + attr->loc;
	write_page.nbyte = attr->nbyte;
	memcpy(write_page.buf, attr->buffer, 256);
	return dev->driver.ioctl(&(dev->handle), I_MEM_WRITEPAGE, &write_page);
}

int appfs_util_priv_create(const devfs_device_t * dev, appfs_handle_t * h, appfs_installattr_t * attr){
	int code_start_addr;
	int type;
	int len;
	int page;
	appfs_file_t * dest;
	dest = (appfs_file_t*)attr->buffer;

	if ( h->is_install == false ){
		errno = EBADF;
		return -1;
	}

	if ( attr->loc == 0 ){

		//This is the header data -- make sure it is complete
		if ( attr->nbyte < sizeof(appfs_file_t) ){
			errno = ENOTSUP;
			return -1;
		}


		if( dest->exec.signature != APPFS_CREATE_SIGNATURE ){
			errno = EINVAL;
			return -3;
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

		//find space for the code
		code_start_addr = find_protectable_free(dev, type, dest->exec.code_size, &page);
		if ( code_start_addr == -1 ){
			//errno is set to ENOSPC by find_protectable_free
			return -1;
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
			errno = ENOTSUP;
			return -1;
		}
	}


	//now write the buffer
	return mem_write_page(dev, h, attr);
}



int appfs_util_priv_writeinstall(const devfs_device_t * dev, appfs_handle_t * h, appfs_installattr_t * attr){
	union {
		const appfs_file_t * file;
		const u32 * ptr;
	} src;
	int i;
	int code_start_addr;
	int code_size;
	int data_start_addr;
	int ram_size;
	int type;
	int len;
	int code_page;
	int ram_page;
	s32 loc_err = 0;

	if ( h->is_install == false ){
		errno = EBADF;
		return -1;
	}

	union {
		appfs_file_t file;
		u32 buf[attr->nbyte>>2];
	} dest;

	src.ptr = (const u32*)attr->buffer;

	if ( attr->loc == 0 ){
		//This is the header data -- make sure it is complete
		if ( attr->nbyte < sizeof(appfs_file_t) ){
			errno = ENOTSUP;
			return -1;
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
			errno = ENOEXEC;
			return -1;
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
		code_start_addr = find_protectable_free(dev, type, code_size, &code_page);
		if ( code_start_addr == -1 ){
			return -1;
		}

		if ( !((src.file->exec.o_flags) & APPFS_FLAG_IS_FLASH) ){ //for RAM app's mark the RAM usage
			//mark the range as SYS
			appfs_ram_setrange(appfs_ram_usagetable,
					code_page,
					code_size,
					APPFS_MEMPAGETYPE_SYS);

			//mark the first page as USER
			appfs_ram_setrange(appfs_ram_usagetable,
					code_page,
					DEVICE_RAM_PAGE_SIZE, //mark the first page as USER
					APPFS_MEMPAGETYPE_USER);
		}

		ram_page = 0;
		data_start_addr = find_protectable_free(dev, MEM_FLAG_IS_RAM, ram_size, &ram_page);
		if( data_start_addr == -1 ){
			if ( !((src.file->exec.o_flags) & APPFS_FLAG_IS_FLASH) ){ //for RAM app's mark the RAM usage
				//free the code section
				appfs_ram_setrange(appfs_ram_usagetable,
						code_page,
						code_size,
						APPFS_MEMPAGETYPE_FREE);

			}
			return -1;
		}

		h->type.install.code_start = (u32)code_start_addr;
		h->type.install.code_size = code_size;
		h->type.install.data_start = (u32)data_start_addr;
		h->type.install.data_size = src.file->exec.data_size;

		h->type.install.rewrite_mask = (u32)(src.file->exec.code_start) & APPFS_REWRITE_MASK;
		h->type.install.kernel_symbols_total = symbols_total();

		appfs_ram_setrange(appfs_ram_usagetable,
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

		errno = 0;
		for(i=sizeof(appfs_file_t) >> 2; i < attr->nbyte >> 2; i++){
			dest.buf[i] = translate_value(src.ptr[i],
					h->type.install.rewrite_mask,
					h->type.install.code_start,
					h->type.install.data_start,
					h->type.install.kernel_symbols_total,
					&loc_err);
			if( loc_err != 0 ){
				errno = EIO;
				return -1 - loc_err;
			}
		}

	} else {

		if ( (attr->loc & 0x03) ){
			//this is not a word aligned write
			errno = ENOTSUP;
			return -1;
		}
		for(i=0; i < attr->nbyte >> 2; i++){
			dest.buf[i] = translate_value(src.ptr[i],
					h->type.install.rewrite_mask,
					h->type.install.code_start,
					h->type.install.data_start,
					h->type.install.kernel_symbols_total,
					&loc_err);
			if( loc_err != 0 ){
				errno = EIO;
				return -1 - loc_err;
			}
		}
	}

	memcpy(attr->buffer, &dest, attr->nbyte);

	//now write buffer
	return mem_write_page(dev, h, attr);
}


int appfs_ram_setusage(int page, int size, int type){
	u32 buf[APPFS_RAM_USAGE_WORDS];
	memcpy(buf, appfs_ram_usagetable, APPFS_RAM_USAGE_BYTES);
	appfs_ram_setrange(buf, page, size, type);
	cortexm_svcall(appfs_ram_priv_saveusage, buf);
	return 0;
}

int appfs_priv_ram_setusage(int page, int size, int type){
	u32 buf[APPFS_RAM_USAGE_WORDS];
	memcpy(buf, appfs_ram_usagetable, APPFS_RAM_USAGE_BYTES);
	appfs_ram_setrange(buf, page, size, type);
	appfs_ram_priv_saveusage(buf);
	return 0;
}

int appfs_util_getflashpagetype(appfs_header_t * info){
	int len;
	int i;
	len = strnlen(info->name, NAME_MAX-2);
	if ( (len == NAME_MAX - 2) || //check if the name is short enough
			(len != strspn(info->name, sysfs_validset)) || //check if only valid characters are present
			(info->name[NAME_MAX-1] != calc_checksum(info->name)) || //check for the second terminating zero
			(len == 0)
	){
		for(i=0; i < NAME_MAX; i++){
			if ( info->name[i] != 0xFF){
				break;
			}
		}

		if (i == NAME_MAX ){
			return APPFS_MEMPAGETYPE_FREE;
		} else {
			return APPFS_MEMPAGETYPE_SYS;
		}
	}

	return APPFS_MEMPAGETYPE_USER;
}

int appfs_util_getpagetype(appfs_header_t * info, int page, int type){
	if ( type == MEM_FLAG_IS_FLASH ){
		return appfs_util_getflashpagetype(info);
	}
	return appfs_ram_getusage(page);
}

int get_hdrinfo(appfs_file_t * file, int page, int type){
	char hex_num[9];
	int page_type;
	page_type = appfs_util_getpagetype(&file->hdr, page, type);
	htoa(hex_num, page);
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

bool appfs_util_isexecutable(const appfs_file_t * info){
	// do a priv read of the signature
	if ( info->exec.signature != symbols_table[0] ){
		return false;
	}
	return true;
}

void appfs_util_privloadfileinfo(void * args){
	priv_load_fileinfo_t * p = (priv_load_fileinfo_t*)args;
	devfs_async_t op;

	if ( p->dev->driver.ioctl(
			&(p->dev->handle),
			I_MEM_GETPAGEINFO,
			&p->pageinfo
	) < 0 ){
		p->ret = -1;
		return;
	}

	op.buf = &(p->fileinfo);
	op.nbyte = sizeof(appfs_file_t);
	op.handler.context = NULL;
	op.loc = (int)p->pageinfo.addr;
	op.tid = task_get_current();

	if ( p->dev->driver.read(&(p->dev->handle), &op) == sizeof(appfs_file_t) ){
		//read successfully
		p->ret = 0;
		return;
	}
	//this is an EIO error
	p->ret = -2;
}

int appfs_util_getfileinfo(priv_load_fileinfo_t * info, const devfs_device_t * dev, int page, int type, int * size){
	int filetype;
	info->dev = dev;
	info->pageinfo.num = page;
	info->pageinfo.o_flags = type;
	cortexm_svcall(appfs_util_privloadfileinfo, info);
	if ( info->ret < 0 ){
		return -1;
	}

	//get the header info for free and sys files
	filetype = get_hdrinfo(&(info->fileinfo), page, type);

	if ( size != NULL ){
		*size = get_filesize(dev, info, filetype);
	}

	return filetype;
}

int get_filesize(const devfs_device_t * dev, priv_load_fileinfo_t * args, int filetype){
	//this will start at the end of the page and count backwards until it hits a non 0xFF value
	if ( filetype == APPFS_MEMPAGETYPE_USER ){
		return args->fileinfo.exec.code_size + args->fileinfo.exec.data_size;
	} else {
		return args->pageinfo.size;
	}
}

int appfs_util_lookupname(const void * cfg, const char * path, priv_load_fileinfo_t * args, int type, int * size){
	int i;

	i = 0;

	if ( strnlen(path, NAME_MAX-2) == NAME_MAX-2 ){
		errno = ENAMETOOLONG;
		return -1;
	}

	do {
		//go through each page
		if ( appfs_util_getfileinfo(args, cfg, i, type, size) < 0){
			return -1;
		}

		if ( strncmp(path, args->fileinfo.hdr.name, NAME_MAX) == 0 ){
			args->ret = i;
			return 0;
		}
		i++;
	} while(1);

	//name not found
	return -1;
}




