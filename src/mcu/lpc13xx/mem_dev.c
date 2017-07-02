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
#include "mcu/mem.h"
#include "mcu/debug.h"
#include "mcu/core.h"

extern uint32_t _text;
extern uint32_t _etext;
extern uint32_t _data;
extern uint32_t _edata;

extern uint32_t _flash_pages;
extern uint32_t _flash_size;
extern uint32_t _sram_size;
extern uint32_t _ahb_sram_size;

#define LAST_BOOTLOADER_PAGE 0

#define START_OF_SRAM 0x10000000
#define END_OF_SRAM ((int)&_sram_size + START_OF_SRAM)
#define SRAM_PAGES (((int)&_sram_size) / DEVICE_RAM_PAGE_SIZE)
#define AHB_SRAM_PAGES ((int)&_ahb_sram_size / DEVICE_RAM_PAGE_SIZE)

#define START_OS ((int)&_text)
#define END_OS ((int)&_etext + (int)&_edata - (int)&_data)

#define FLASH_SIZE ((int)&_flash_size)


static int blank_check(int loc, int nbyte);
static bool is_flash(int addr, int size);
static int get_flash_page(int addr);
static int get_flash_page_size(int page);
static int get_flash_page_addr(int page);

static bool is_ram(int addr, int size);
static int get_ram_page(int addr);
static int get_ram_page_size(int page);
static int get_ram_page_addr(int page);

void _mcu_mem_dev_power_on(int port){}
void _mcu_mem_dev_power_off(int port){}
int _mcu_mem_dev_powered_on(int port){
	return 1;
}

int _mcu_mem_getsyspage(){
	return (SRAM_PAGES);
}

int mcu_mem_getattr(int port, void * ctl){
	mem_attr_t * attr = ctl;
	attr->flash_pages = (uint32_t)&_flash_pages;
	attr->flash_size = (uint32_t)&_flash_size;
	attr->ram_pages = (uint32_t)&_flash_pages;
	attr->ram_size = (uint32_t)&_sram_size + (uint32_t)&_ahb_sram_size;
	return 0;
}
int mcu_mem_setattr(int port, void * ctl){
	return 0;
}

int mcu_mem_setaction(int port, void * ctl){
	errno = ENOTSUP;
	return -1;
}

int mcu_mem_getpageinfo(int port, void * ctl){
	uint32_t size = 0;
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

int mcu_mem_erasepage(int port, void * ctl){
	int err;
	int addr;
	uint32_t page;
	page = (uint32_t)ctl;
	addr = get_flash_page_addr(page);  //this gets the beginning of the page


	//protect the OS and the bootloader from being erased
	if ( (page < LAST_BOOTLOADER_PAGE) ||
			((addr >= START_OS) && (addr <= END_OS)) ){
		errno = EROFS;
		return -1;
	}

	_mcu_cortexm_priv_disable_interrupts(NULL);
	err = _mcu_lpc_flash_erase_page((uint32_t)ctl);
	_mcu_cortexm_priv_enable_interrupts(NULL);
	if ( err < 0 ){
		errno = EIO;
		return -1;
	}
	return 0;
}

int mcu_mem_writepage(int port, void * ctl){
	int err;
	int nbyte;
	mem_writepage_t * wattr = ctl;
	int boot_page_addr = get_flash_page_addr(LAST_BOOTLOADER_PAGE);

	if ( is_ram(wattr->addr, wattr->nbyte) ){
		memcpy((void*)wattr->addr, wattr->buf, wattr->nbyte);
		return wattr->nbyte;
	}


	if ( wattr->addr < boot_page_addr ){
		errno = EROFS;
		return -1;
	}

	if ( wattr->addr >= (uint32_t)&_flash_size ){
		return -1;
	}

	if ( wattr->addr + wattr->nbyte > (uint32_t)&_flash_size ){
		wattr->nbyte = (uint32_t)&_flash_size - wattr->addr; //update the bytes read to not go past the end of the disk
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


	err = _mcu_lpc_flash_write_page(get_flash_page(wattr->addr), (void*)wattr->addr, wattr->buf, nbyte);
	if( err < 0 ){
		errno = EIO;
		return -1;
	}

	return wattr->nbyte;
}

int _mcu_mem_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){

	if ( is_ram(wop->loc, wop->nbyte) ){
		memcpy((void*)wop->loc, wop->buf, wop->nbyte);
		return wop->nbyte;
	}

	errno = EINVAL;
	return -1;
}

int _mcu_mem_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
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
	if ( (addr + size) <= (int)&_flash_size ){
		return true;
	}
	return false;
}

//RAM paging
int get_ram_page(int addr){
	int page;
	int offset;
	int page_offset;
	if ( (addr > START_OF_SRAM) && (addr < END_OF_SRAM) ){
		offset = START_OF_SRAM;
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
		return START_OF_SRAM + page*DEVICE_RAM_PAGE_SIZE;
	}
	return -1;
}

bool is_ram(int addr, int size){
	if ( (addr >= START_OF_SRAM) && ((addr + size) <= END_OF_SRAM) ){
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
			//mcu_priv_debug("blank check: 0x%X is 0x%X\n", loc + i, locp[i]);
			return -1;
		}
	}
	return 0;
}

