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
#include "mcu/flash.h"
#include "mcu/core.h"


extern uint32_t _text;
extern uint32_t _etext;
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _flash_size;


#define LAST_BOOTLOADER_PAGE 0

static int blank_check(int loc, int nbyte);
static int get_page(void * addr);
static int get_page_size(int page);
static int get_page_addr(int page);

void flash_dev_power_on(int port){}
void flash_dev_power_off(int port){}
int flash_dev_powered_on(int port){
	return 1;
}

int mcu_flash_getinfo(int port, void * ctl){
	return 0;
}
int mcu_flash_setattr(int port, void * ctl){
	return 0;
}

int mcu_flash_setaction(int port, void * ctl){
	return 0;
}

int mcu_flash_getpageinfo(int port, void * ctl){
	uint32_t size;
	uint32_t addr;
	flash_pageinfo_t * ctlp = (flash_pageinfo_t *)ctl;

	size = get_page_size(ctlp->page);
	addr = get_page_addr(ctlp->page);
	if ( (addr + size) > (uint32_t)&_flash_size){
		return -1; //this page does not exist on this part
	}
	ctlp->size = size;
	ctlp->addr = addr;
	return 0;
}


int mcu_flash_eraseaddr(int port, void * ctl){
	int err;
	int page;

	uint32_t data_size;
	data_size = (uint32_t)&_edata - (uint32_t)&_data;

	//Protect the kernel code from being erased
	if ( ((uint32_t)ctl < (uint32_t)&_text) ||
			((uint32_t)ctl > ((uint32_t)&_etext + data_size)) ){
		page = get_page(ctl);
		err = mcu_flash_erasepage(port, (void*)page);
		return err;
	}
	errno = EROFS;
	return -1;
}

int mcu_flash_erasepage(int port, void * ctl){
	int err;
	uint32_t page;
	page = (uint32_t)ctl;



	if ( page < LAST_BOOTLOADER_PAGE ){
		//Never erase the bootloader
		errno = EPERM;
		return -1;
	}

	int addr;
	int page_size;
	addr = get_page_addr(page);
	page_size = get_page_size(page);

	if ( (addr + page_size) > (uint32_t)&_flash_size){
		return -1; //this page does not exist on this part
	}

	if ( blank_check(addr, page_size) == 0 ){
		return 0;
	}

	_mcu_cortexm_priv_disable_interrupts(NULL);
	err = _mcu_lpc_flash_erase_page((uint32_t)ctl);
	_mcu_cortexm_priv_enable_interrupts(NULL);
	if ( err < 0 ){
		errno = EIO;
	}
	return err;
}

int mcu_flash_getpage(int port, void * ctl){
	return get_page(ctl);
}

int mcu_flash_getsize(int port, void * ctl){
	return (int)&_flash_size;
}

int blank_check(int loc, int nbyte){
	int i;
	const int8_t * locp;
	//Do a blank check
	locp = (const int8_t*)loc;
	for(i = 0; i < nbyte; i++){
		if ( (locp)[i] != -1 ){
			return -1;
		}
	}
	return 0;
}

int mcu_flash_writepage(int port, void * ctl){
	int err;
	int nbyte;
	flash_writepage_t * wattr = ctl;
	int boot_page_addr = get_page_addr(LAST_BOOTLOADER_PAGE);


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


	err = _mcu_lpc_flash_write_page(get_page((void*)wattr->addr), (void*)wattr->addr, wattr->buf, nbyte);
	if( err < 0 ){
		errno = EIO;
		return -1;
	}

	return wattr->nbyte;

}

int _mcu_flash_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){

	if ( rop->loc >= (uint32_t)&_flash_size ){
		return -1;
	}

	if ( rop->loc + rop->nbyte > (uint32_t)&_flash_size ){
		rop->nbyte = (uint32_t)&_flash_size - rop->loc; //update the bytes read to not go past the end of the disk
	}

	memcpy(rop->buf, (const void*)rop->loc, rop->nbyte);
	return rop->nbyte;
}

//Get the flash page that contains the address
int get_page(void * addr){
	int iaddr = (int)addr;
	if ( iaddr < 0x10000 ){
		return iaddr / 4096;
	}
	iaddr -= 0x10000;
	return 16 + iaddr / 32768;
}

int get_page_size(int page){
	if ( page < 16 ){
		return 4*1024;
	} else {
		return 32*1024;
	}
}

int get_page_addr(int page){
	if ( page < 16 ){
		return page * 4096;
	} else {
		return (16*4096) + ((page-16)*32*1024);
	}
}
