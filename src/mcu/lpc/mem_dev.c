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

#include "config.h"
#include <errno.h>
#include "mcu/cortexm.h"
#include "mcu/mem.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/wdt.h"
#include "mcu/bootloader.h"


extern u32 _text;
extern u32 _etext;
extern u32 _data;
extern u32 _edata;

extern u32 _flash_pages;
extern u32 _flash_size;
extern u32 _sram_size;
extern u32 _ahb_sram_size;

#define END_OF_SRAM ((int)&_sram_size + MCU_START_OF_SRAM)
#define END_OF_AHB_SRAM ((int)&_ahb_sram_size + MCU_START_OF_AHB_SRAM)
#define SRAM_PAGES (((int)&_sram_size) / DEVICE_RAM_PAGE_SIZE)
#define AHB_SRAM_PAGES ((int)&_ahb_sram_size / DEVICE_RAM_PAGE_SIZE)

#define START_OS ((int)&_text)
#define END_OS ((int)&_etext + (int)&_edata - (int)&_data)

#define FLASH_SIZE ((u32)&_flash_size)

static int blank_check(int loc, int nbyte);
static bool is_flash(int addr, int size);
static int get_flash_page(int addr);
static int get_flash_page_size(int page);
static int get_flash_page_addr(int page);

static int get_last_boot_page();

static bool is_ram(int addr, int size);
static int get_ram_page(int addr);
static int get_ram_page_size(int page);
static int get_ram_page_addr(int page);

void mcu_mem_dev_power_on(const devfs_handle_t * handle){}
void mcu_mem_dev_power_off(const devfs_handle_t * handle){}
int mcu_mem_dev_is_powered(const devfs_handle_t * handle){
	return 1;
}

int mcu_mem_getsyspage(){
	return (SRAM_PAGES);
}

int mcu_mem_getinfo(const devfs_handle_t * handle, void * ctl){
	mem_attr_t * attr = ctl;
	attr->flash_pages = (u32)&_flash_pages;
	attr->flash_size = FLASH_SIZE;
	attr->ram_pages = (u32)&_flash_pages;
	attr->ram_size = (u32)&_sram_size + (u32)&_ahb_sram_size;
	return 0;
}
int mcu_mem_setattr(const devfs_handle_t * handle, void * ctl){
	return 0;
}

int mcu_mem_setaction(const devfs_handle_t * handle, void * ctl){
	errno = ENOTSUP;
	return -1;
}

int mcu_mem_getpageinfo(const devfs_handle_t * handle, void * ctl){
	u32 size = 0;
	int32_t addr = 0;
	mem_pageinfo_t * ctlp = ctl;

	switch(ctlp->type){
	case MEM_PAGEINFO_TYPE_QUERY:
		//lookup the page info based on the address
		if ( is_ram(ctlp->addr, DEVICE_RAM_PAGE_SIZE) ){
			ctlp->num = get_ram_page(ctlp->addr);
			ctlp->size = get_ram_page_size(ctlp->num);
			ctlp->type = MEM_PAGEINFO_TYPE_RAM;
			return 0;
		} else if ( is_flash(ctlp->addr, 0) ){
			ctlp->num = get_flash_page(ctlp->addr);
			ctlp->size = get_flash_page_size(ctlp->num);
			ctlp->type = MEM_PAGEINFO_TYPE_FLASH;
			return 0;
		}

		break;
	case MEM_PAGEINFO_TYPE_RAM:
		//get the ram page info
		size = get_ram_page_size(ctlp->num);
		addr = get_ram_page_addr(ctlp->num);
		if ( addr < 0 ){
			return -1;
		}
		break;
	case MEM_PAGEINFO_TYPE_FLASH:
		//get the flash page info
		size = get_flash_page_size(ctlp->num);
		addr = get_flash_page_addr(ctlp->num);
		if ( (addr + size) > FLASH_SIZE){
			return -1; //this page does not exist on this part
		}
		break;
	default:
		return -1;

	}

	ctlp->size = size;
	ctlp->addr = addr;
	return 0;
}



int mcu_mem_erasepage(const devfs_handle_t * handle, void * ctl){
	int err;
	int addr;
	int page;
	page = (u32)ctl;
	addr = get_flash_page_addr(page);  //this gets the beginning of the page
	int last_boot_page = get_last_boot_page();

	//protect the OS and the bootloader from being erased
	if ( (page <= last_boot_page) ||
			((addr >= START_OS) && (addr <= END_OS)) ){
		errno = EROFS;
		return -1;
	}

	mcu_cortexm_priv_disable_interrupts(NULL);
	err = mcu_lpc_flash_erase_page((u32)ctl);
	mcu_cortexm_priv_enable_interrupts(NULL);
	if ( err < 0 ){
		errno = EIO;
		return -1;
	}
	return 0;
}

int mcu_mem_writepage(const devfs_handle_t * handle, void * ctl){
	int err;
	int nbyte;
	mem_writepage_t * wattr = ctl;
	int last_boot_page;

	if ( is_ram(wattr->addr, wattr->nbyte) ){
		memcpy((void*)wattr->addr, wattr->buf, wattr->nbyte);
		return wattr->nbyte;
	}

	last_boot_page = get_last_boot_page();

	if ( wattr->addr <= last_boot_page ){
		errno = EROFS;
		return -1;
	}

	if ( wattr->addr >= FLASH_SIZE ){
		errno = EINVAL;
		return -11;
	}

	if ( wattr->addr + wattr->nbyte > FLASH_SIZE ){
		wattr->nbyte = FLASH_SIZE - wattr->addr; //update the bytes read to not go past the end of the disk
	}

	if ( wattr->nbyte <= 256 ){
		nbyte = 256;
	} else if( wattr->nbyte <= 512 ){
		nbyte = 512;
	} else if ( wattr->nbyte <= 1024 ){
		nbyte = 1024;
	} else if ( wattr->nbyte > 1024 ){
		nbyte = 1024;
	}


	if ( blank_check(wattr->addr,  nbyte) ){
		errno = EROFS;
		return -1;
	}


	err = mcu_lpc_flash_write_page(get_flash_page(wattr->addr), (void*)wattr->addr, wattr->buf, nbyte);
	if( err < 0 ){
		errno = EIO;
		return -1;
	}

	return wattr->nbyte;
}

int mcu_mem_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){

	if ( is_ram(wop->loc, wop->nbyte) ){
		memcpy((void*)wop->loc, wop->buf, wop->nbyte);
		return wop->nbyte;
	}

	errno = EINVAL;
	return -1;
}

int mcu_mem_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	if ( (is_flash(rop->loc, rop->nbyte) ) ||
			( is_ram(rop->loc, rop->nbyte) ) 	){
		memcpy(rop->buf, (const void*)rop->loc, rop->nbyte);
		return rop->nbyte;
	}
	errno = EINVAL;
	return -1;
}

//Get the flash page that contains the address
int get_flash_page(int addr){
	if ( addr < 0x10000 ){
		return addr / 4096;
	}
	addr -= 0x10000;
	return 16 + addr / 32768;
}

int get_flash_page_size(int page){
	if ( page < 16 ){
		return 4*1024;
	} else {
		return 32*1024;
	}
}

int get_flash_page_addr(int page){
	if ( page < 16 ){
		return page * 4096;
	} else {
		return (16*4096) + ((page-16)*32*1024);
	}
}

bool is_flash(int addr, int size){
	if ( (addr + size) <= FLASH_SIZE ){
		return true;
	}
	return false;
}

//RAM paging
int get_ram_page(int addr){
	int page;
	int offset;
	int page_offset;
	if ( (addr >= MCU_START_OF_AHB_SRAM) && (addr < END_OF_AHB_SRAM) ){
		offset = MCU_START_OF_AHB_SRAM;
		page_offset = SRAM_PAGES;
	} else if ( (addr >= MCU_START_OF_SRAM) && (addr < END_OF_SRAM) ){
		offset = MCU_START_OF_SRAM;
		page_offset = 0;
	} else {
		return -1;
	}
	page = (addr - offset) / DEVICE_RAM_PAGE_SIZE + page_offset;
	return page;
}

int get_ram_page_size(int page){
	int size;
	size = DEVICE_RAM_PAGE_SIZE;
	return size;
}

int get_ram_page_addr(int page){
	if ( page < SRAM_PAGES ){
		return MCU_START_OF_SRAM + page*DEVICE_RAM_PAGE_SIZE;
	} else if ( page < (SRAM_PAGES + AHB_SRAM_PAGES)){
		return MCU_START_OF_AHB_SRAM + (page - SRAM_PAGES)*DEVICE_RAM_PAGE_SIZE;
	}

	return -1;
}

bool is_ram(int addr, int size){
	if ( (addr >= MCU_START_OF_SRAM) && ((addr + size) <= END_OF_SRAM) ){
		return true;
	}

	if ( (addr >= MCU_START_OF_AHB_SRAM) && ((addr + size) <= END_OF_AHB_SRAM) ){
		return true;
	}

	return false;
}

int blank_check(int loc, int nbyte){
	int i;
	const int8_t * locp;
	//Do a blank check
	locp = (const int8_t*)loc;
	for(i = 0; i < nbyte; i++){
		if ( locp[i] != -1 ){
			return -1;
		}
	}
	return 0;
}

int get_last_boot_page(){
	bootloader_api_t api;
	mcu_core_priv_bootloader_api(&api);

	if( api.code_size > 0 ){ //zero means there is not bootloader installed
		return get_flash_page(api.code_size);
	}

	return -1;

}

