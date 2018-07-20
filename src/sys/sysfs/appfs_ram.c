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

#include "sos/sos.h"
#include "cortexm/mpu.h"
#include "appfs_local.h"

static void set_ram_usage(u32 * buf, u32 page, int usage);

int appfs_ram_setusage(u32 page, u32 size, int type){
	u32 buf[APPFS_RAM_USAGE_WORDS_SIZE];
	appfs_ram_getusage_all(buf);
	appfs_ram_setrange(buf, page, size, type);
	cortexm_svcall(appfs_ram_root_saveusage, buf);
	return 0;
}

int appfs_ram_root_setusage(u32 page, u32 size, int type){
	u32 buf[APPFS_RAM_USAGE_WORDS_SIZE];
	appfs_ram_getusage_all(buf);
	appfs_ram_setrange(buf, page, size, type);
	appfs_ram_root_saveusage(buf);
	return 0;
}


void appfs_ram_root_saveusage(void * args){
	memcpy(mcu_ram_usage_table, args, APPFS_RAM_USAGE_WORDS_SIZE*sizeof(u32));
}

void appfs_ram_setrange(u32 * buf, u32 page, u32 size, int usage){
    u32 i;
    u32 pages;
    size = (u32)mpu_getnextpowerof2((int)size);
	pages = (size + MCU_RAM_PAGE_SIZE - 1) / MCU_RAM_PAGE_SIZE;
	for(i=page; i < (page+pages); i++){
		set_ram_usage(buf, i, usage);
	}
}

int appfs_ram_getusage(u32 page){
	int block;
	int shift;
	if ( (u32)page < APPFS_RAM_PAGES){
		block = page >> 4;
		shift = (page & 0xF) * 2;
		return ( (mcu_ram_usage_table[block] >> (shift)) & APPFS_MEMPAGETYPE_MASK );
    }

    return -1;
}

void appfs_ram_getusage_all(void * buf){
	memcpy(buf, mcu_ram_usage_table, APPFS_RAM_USAGE_WORDS_SIZE*sizeof(u32));
}

void appfs_ram_initusage(void * buf){
	memset(buf, 0, APPFS_RAM_USAGE_WORDS_SIZE*sizeof(u32));
}

static void set_ram_usage(u32 * buf, u32 page, int usage){
	u32 block;
	u32 shift;
	if( page < APPFS_RAM_PAGES ){
		block = page >> 4;
		shift = (page & 0xF) * 2;
        buf[block] &= (u32)~(APPFS_MEMPAGETYPE_MASK << (shift)); //clear the bits
        buf[block] |= (u32)((usage & APPFS_MEMPAGETYPE_MASK) << (shift));  //set the bits
	}
}




