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

#include "sos/sos.h"
#include "cortexm/mpu.h"
#include "appfs_local.h"


void appfs_ram_priv_saveusage(void * args){
	memcpy(sos_appfs_ram_usage_table, args, APPFS_RAM_PAGES);
}

//this is a privileged call
static void set_ram_usage(uint32_t * buf, int page, int usage){
	u32 block;
	u32 shift;
	block = page >> 4;
	shift = (page & 0xF) * 2;
	buf[block] &= ~(APPFS_MEMPAGETYPE_MASK << (shift)); //clear the bits
	buf[block] |= ((usage & APPFS_MEMPAGETYPE_MASK) << (shift));  //set the bits
}

void appfs_ram_setrange(uint32_t * buf, int page, int size, int usage){
	int i;
	int pages;
	size = mpu_getnextpowerof2(size);
	pages = (size + MCU_RAM_PAGE_SIZE - 1) / MCU_RAM_PAGE_SIZE;
	for(i=page; i < (pages+page); i++){
		set_ram_usage(buf, i, usage);
	}
}

int appfs_ram_getusage(int page){
	int block;
	int shift;
	if ( (uint32_t)page < APPFS_RAM_PAGES){
		block = page / 16;
		shift = (page % 16) * 2;
		return ( (sos_appfs_ram_usage_table[block] >> (shift)) & APPFS_MEMPAGETYPE_MASK );
	} else {
		return -1;
	}
}



