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

#include <stddef.h>

#include "sffs_filelist.h"
#include <sys/sffs/sffs_list.h>


#define DEBUG_LEVEL 10


int sffs_filelist_isfree(void * data){
	sffs_filelist_item_t * item;
	item = (sffs_filelist_item_t*)data;

	if ( item->status == SFFS_FILELIST_STATUS_FREE ){
		return 1;
	} else {
		return 0;
	}
}

static int is_dirty(void * data){
	sffs_filelist_item_t * item;
	item = (sffs_filelist_item_t*)data;
	if ( item->status == SFFS_FILELIST_STATUS_DIRTY ){
		return 1;
	} else {
		return 0;
	}
}


block_t sffs_filelist_get(const void * cfg, block_t list_block, int segment, uint8_t status, int * addr){
	sffs_list_t list;
	sffs_filelist_item_t item;

	if( sffs_filelist_init(cfg, &list, list_block) < 0 ){
		sffs_debug(DEBUG_LEVEL, "list block is invalid\n");
		return BLOCK_INVALID;
	}

	while( sffs_list_getnext(cfg, &list, &item, addr) == 0 ){
		sffs_debug(DEBUG_LEVEL + 3, "block:%d segment:%d = %d?\n", item.block, item.segment, segment);
		if ( (item.status == status) && (item.segment == segment) ){
			return item.block;
		}
	}

	return BLOCK_INVALID;
}

int sffs_filelist_makeobsolete(const void * cfg, block_t list_block){
	sffs_list_t list;
	sffs_filelist_item_t item;
	int addr;

	if( sffs_filelist_init(cfg, &list, list_block) < 0 ){
		sffs_debug(DEBUG_LEVEL, "list block is invalid\n");
		return BLOCK_INVALID;
	}

	while( sffs_list_getnext(cfg, &list, &item, &addr) == 0 ){
		if ( item.status == SFFS_FILELIST_STATUS_CURRENT){
			if (sffs_filelist_setstatus(cfg, SFFS_FILELIST_STATUS_OBSOLETE, addr) < 0 ){
				sffs_error("failed to mark entry as obsolete\n");
				return -1;
			}
		}
	}
	return 0;
}

int sffs_filelist_setstatus(const void * cfg, uint8_t status, int addr){
	if ( sffs_dev_write(cfg, addr + offsetof(sffs_filelist_item_t, status), &status, sizeof(status)) != sizeof(status) ){
		return -1;
	}
	return 0;
}

int sffs_filelist_update(const void * cfg, block_t list_block, int segment, block_t new_block){
	sffs_filelist_item_t item;
	sffs_list_t list;
	int addr;
	int strike_addr;
	int entry_num;

	if ( sffs_filelist_init(cfg, &list, list_block) < 0 ){
		return -1;
	}

	strike_addr = -1;
	entry_num = 0;
	while( sffs_list_getnext(cfg, &list, &item, &addr) == 0 ){
		//is this the one to strike??
		if( (item.segment == segment) && (item.status == SFFS_FILELIST_STATUS_CURRENT) ){
			strike_addr = addr;
			sffs_debug(DEBUG_LEVEL, "striking segment %d block %d entry %d\n", item.segment, item.block, entry_num);
			break;
		}
		entry_num++;
	}


	sffs_debug(DEBUG_LEVEL, "Appending entry %d segment %d\n", entry_num, segment);
	//The serial number is at the head of every block
	item.status = SFFS_FILELIST_STATUS_CURRENT;
	item.block = new_block; //The block location of the segment
	item.segment = segment; //The segment number in the file (unique)
	if( sffs_list_append(cfg, &list, BLOCK_TYPE_FILE_LIST, &item, NULL) < 0 ){
		return -1;
	}

	//mark the old entry as obsolete
	if ( strike_addr != -1 ){
		if( sffs_filelist_setstatus(cfg, SFFS_FILELIST_STATUS_OBSOLETE, strike_addr) < 0 ){
			return -1;
		}
	}

	return 0;
}

block_t sffs_filelist_consolidate(const void * cfg, serial_t serialno, block_t list_block){
	return sffs_list_consolidate(cfg, serialno, list_block, BLOCK_TYPE_FILE_LIST, sizeof(sffs_filelist_item_t), is_dirty, sffs_filelist_isfree);
}
