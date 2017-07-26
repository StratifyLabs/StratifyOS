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
#include "cortexm/cortexm.h"
#include "mcu/flash.h"
#include "mcu/core.h"


extern u32 _text;
extern u32 _etext;
extern u32 _data;
extern u32 _edata;
extern u32 _flash_size;

static int blank_check(int loc, int nbyte);
static int get_page(void * addr);
static int get_page_size(int page);
static int get_page_addr(int page);


static int get_last_bootloader_page(){
	return ((int)(&_etext) + 4095)/4096;
}

void flash_dev_power_on(const devfs_handle_t * handle){}
void flash_dev_power_off(const devfs_handle_t * handle){}
int flash_dev_is_powered(const devfs_handle_t * handle){
	return 1;
}

int mcu_flash_getinfo(const devfs_handle_t * handle, void * ctl){
	return 0;
}
int mcu_flash_setattr(const devfs_handle_t * handle, void * ctl){
	return 0;
}

int mcu_flash_setaction(const devfs_handle_t * handle, void * ctl){
	return 0;
}

int mcu_flash_getpageinfo(const devfs_handle_t * handle, void * ctl){
	u32 size;
	u32 addr;
	flash_pageinfo_t * ctlp = (flash_pageinfo_t *)ctl;

	size = get_page_size(ctlp->page);
	addr = get_page_addr(ctlp->page);
	if ( (addr + size) > (u32)&_flash_size){
		return -1; //this page does not exist on this part
	}
	ctlp->size = size;
	ctlp->addr = addr;
	return 0;
}


int mcu_flash_eraseaddr(const devfs_handle_t * handle, void * ctl){
	int err;
	int page;

	u32 data_size;
	data_size = (u32)&_edata - (u32)&_data;

	//Protect the kernel code from being erased
	if ( ((u32)ctl < (u32)&_text) ||
			((u32)ctl > ((u32)&_etext + data_size)) ){
		page = get_page(ctl);
		err = mcu_flash_erasepage(handle, (void*)page);
		return err;
	}
	errno = EROFS;
	return -1;
}


int mcu_flash_erasepage(const devfs_handle_t * handle, void * ctl){
	int err;
	int addr;
	int page_size;
	u32 page;
	page = (u32)ctl;
	int last_page = get_last_bootloader_page();

	if ( page < last_page ){
		//Never erase the bootloader
		errno = EROFS;
		return -1;
	}

	addr = get_page_addr(page);
	page_size = get_page_size(page);

	if ( (addr + page_size) > (u32)&_flash_size){
		errno = EINVAL;
		return -1; //this page does not exist on this part
	}

	if ( blank_check(addr, page_size) == 0 ){
		return 0;
	}

	cortexm_disable_interrupts(NULL);
	err = mcu_lpc_flash_erase_page((u32)ctl);
	cortexm_enable_interrupts(NULL);
	if ( err < 0 ){
		errno = EIO;
	}
	return err;
}

int mcu_flash_getpage(const devfs_handle_t * handle, void * ctl){
	return get_page(ctl);
}

int mcu_flash_getsize(const devfs_handle_t * handle, void * ctl){
	return (int)&_flash_size;
}

int blank_check(int loc, int nbyte){
	int i;
	const s8 * locp;
	//Do a blank check
	locp = (const int8_t*)loc;
	for(i = 0; i < nbyte; i++){
		if ( (locp)[i] != -1 ){
			return -1;
		}
	}
	return 0;
}

int mcu_flash_writepage(const devfs_handle_t * handle, void * ctl){
	int err;
	int nbyte;
	flash_writepage_t * wattr = ctl;
	int boot_page_addr = get_page_addr(get_last_bootloader_page());


	if ( wattr->addr < boot_page_addr ){
		errno = EROFS;
		return -1;
	}

	if ( wattr->addr >= (u32)&_flash_size ){
		return -1;
	}

	if ( wattr->addr + wattr->nbyte > (u32)&_flash_size ){
		wattr->nbyte = (u32)&_flash_size - wattr->addr; //update the bytes read to not go past the end of the disk
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


	err = mcu_lpc_flash_write_page(get_page((void*)wattr->addr), (void*)wattr->addr, wattr->buf, nbyte);
	if( err < 0 ){
		errno = EIO;
		return -1;
	}

	return wattr->nbyte;

}

int mcu_flash_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){

	if ( rop->loc >= (u32)&_flash_size ){
		return -1;
	}

	if ( rop->loc + rop->nbyte > (u32)&_flash_size ){
		rop->nbyte = (u32)&_flash_size - rop->loc; //update the bytes read to not go past the end of the disk
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
