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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sffs_block.h"
#include <sys/sffs/sffs_list.h>

#define DEBUG_LEVEL 10

static int calc_total_items(const void * cfg, int item_size);
static void * get_item(sffs_list_block_t * list, int n, int item_size);
static int get_hdr_addr(const void * cfg, block_t block);
static int get_item_addr(const void * cfg, block_t block, int item, int item_size);
static block_t list_add_block(const void * cfg, serial_t serialno, uint8_t type, block_t list_block);
static int list_update(const void * cfg, sffs_list_t * list, block_t prev_block);
static bool list_typeisvalid(uint8_t type);

bool list_typeisvalid(uint8_t type){
	if ( (type & BLOCK_TYPE_LIST_FLAG) == 0 ){
		return false;
	}
	switch( type & ~(BLOCK_TYPE_LINKED_LIST_FLAG) ){
	case BLOCK_TYPE_SERIALNO_LIST:
	case BLOCK_TYPE_FILE_LIST:
		return true;
	default:
		return false;
	}
}

int calc_total_items(const void * cfg, int item_size){
	return (BLOCK_DATA_SIZE - sizeof(sffs_list_hdr_t)) / item_size;
}

void * get_item(sffs_list_block_t * list, int n, int item_size){
	return list->data + (item_size * n);
}

int get_hdr_addr(const void * cfg, block_t block){
	return block * BLOCK_SIZE + BLOCK_HEADER_SIZE;
}

int get_item_addr(const void * cfg, block_t block, int item, int item_size){
	return block * BLOCK_SIZE + BLOCK_HEADER_SIZE + sizeof(sffs_list_hdr_t) + item * item_size;
}

block_t list_add_block(const void * cfg, serial_t serialno, uint8_t type, block_t list_block){
	sffs_list_hdr_t list_hdr;
	int addr;
	//get a new block and link the new block to this list
	list_hdr.prev = list_block;
	list_hdr.next = sffs_block_alloc(cfg, serialno, list_block, type | BLOCK_TYPE_LINKED_LIST_FLAG);
	if ( list_hdr.next == BLOCK_INVALID ){
		sffs_error("no more blocks\n");
		return BLOCK_INVALID;
	}
	addr = get_hdr_addr(cfg, list_block) + offsetof(sffs_list_hdr_t, next);

	sffs_debug(DEBUG_LEVEL, "next block is %d\n", list_hdr.next);
	//write list->hdr.next to the disk at addr + offsetof(sffs_list_hdr_t, next)
	if ( sffs_dev_write(cfg, addr, &list_hdr.next, sizeof(list_hdr.next)) !=sizeof(list_hdr.next) ){
		sffs_error("failed to write next block\n");
		return BLOCK_INVALID;
	}

	addr = get_hdr_addr(cfg, list_hdr.next) + offsetof(sffs_list_hdr_t, prev);
	sffs_debug(DEBUG_LEVEL, "prev block is %d\n", list_hdr.prev);
	//write list->hdr.next to the disk at addr + offsetof(sffs_list_hdr_t, next)
	if ( sffs_dev_write(cfg, addr, &list_hdr.prev, sizeof(list_hdr.prev)) != sizeof(list_hdr.prev) ){
		sffs_error("failed to write prev block\n");
		return BLOCK_INVALID;
	}

	//if this is for the serial numbers then close the block right away
	if ( type == BLOCK_TYPE_SERIALNO_LIST ){
		if ( sffs_block_close(cfg, list_hdr.next) < 0 ){
			sffs_error("failed to close serialno list block\n");
			return BLOCK_INVALID;
		}
	}

	return list_hdr.next;
}


int list_update(const void * cfg, sffs_list_t * list, block_t prev_block){
	sffs_list_hdr_t * list_hdr;
	list->current_item = 0;
	if( sffs_block_load(cfg, list->current_block, &(list->block_data)) < 0 ){
		sffs_error("failed to laod block %d\n", list->current_block);
		return -1;
	}
	list_hdr = (sffs_list_hdr_t*)list->block_data.data;
	//mcu_debug_user_printf("loaded %d (prev:%d next:%d) 0x%X %d\n", list->current_block, list_hdr->prev, list_hdr->next, list->block_data.hdr.type, list->block_data.hdr.serialno);
	if( (prev_block != list_hdr->prev) || (list_typeisvalid(list->block_data.hdr.type) == false) ){ //this check makes sure the list is consistent
		sffs_error("list has an error\n");
		return -1;
	}
	return 0;
}

int sffs_list_init(const void * cfg, sffs_list_t * list, block_t list_block, int item_size, int (*is_free)(void*)){
	list->total_in_block = calc_total_items(cfg, item_size);
	list->item_size = item_size;
	list->current_block = list_block;
	list->is_free = is_free;
	sffs_debug(DEBUG_LEVEL, "new list of %d items (size:%d)\n", list->total_in_block, item_size);
	return list_update(cfg, list, BLOCK_INVALID);
}

int sffs_list_getnext(const void * cfg, sffs_list_t * list, void * item, int * addr){
	sffs_list_block_t * ptr;
	void * new_item;
	ptr = (sffs_list_block_t*)list->block_data.data;
	block_t current_block;

	if ( list->current_item < list->total_in_block ){
		if ( addr != NULL ){
			*addr = get_item_addr(cfg, list->current_block, list->current_item, list->item_size);
		}
		sffs_debug(DEBUG_LEVEL+4, "item addr: 0x%X\n", get_item_addr(cfg, list->current_block, list->current_item, list->item_size));
		new_item = get_item(ptr, list->current_item, list->item_size);
		if ( item != NULL ){
			memcpy(item, new_item, list->item_size);
		}

		/*
		sffs_debug(DEBUG_LEVEL, "Item contents:");
		for(i=0; i < list->item_size; i++){
			if ( CL_DEBUG >= DEBUG_LEVEL ){
				printf("0x%X ", ((unsigned char*)new_item)[i]);
			}
		}
		if ( CL_DEBUG >= DEBUG_LEVEL ){
			printf("\n");
		}
		 */

		if( list->is_free(new_item) ){
			sffs_debug(DEBUG_LEVEL, "end of list (free entries) %d item(s)\n", list->current_item+1);
			return 1;
		}
		list->current_item++;
	} else {
		return -1;
	}

	if ( list->current_item == list->total_in_block ){
		sffs_debug(DEBUG_LEVEL + 2, "load next block %d\n", ptr->hdr.next);
		if ( ptr->hdr.next != BLOCK_INVALID ){
			current_block = list->current_block;
			list->current_block = ptr->hdr.next;
			if ( current_block == ptr->hdr.next ){
				sffs_error("circular list\n");
				return -1;
			}
			if ( list_update(cfg, list, current_block) < 0 ){
				sffs_error("error loading block %d\n", list->current_block);
				return -1;
			}
		} else {
			sffs_debug(DEBUG_LEVEL, "end of list (none free)\n");
			return 0;
		}
	}

	return 0;
}

int sffs_list_setblockstatus(const void * cfg, block_t list_block, uint8_t status){
	sffs_list_hdr_t hdr;
	block_t tmp_block;
	block_t last_block;
	int i,j;
	int total;

	last_block = BLOCK_INVALID;
	for(tmp_block = list_block; tmp_block != BLOCK_INVALID; tmp_block = hdr.next){

		if ( sffs_dev_read(cfg, get_hdr_addr(cfg, tmp_block), &hdr, sizeof(hdr)) != sizeof(hdr) ){
			sffs_error("failed to read\n");
			return -1;
		}

		if ( hdr.prev != last_block ){
			sffs_error("bad list\n");
			return -1;
		}

		sffs_debug(DEBUG_LEVEL, "this block is %d (prev:%d next:%d)\n", tmp_block, hdr.prev, hdr.next);
		last_block = tmp_block;
		i++;
	}

	total = 0;
	for(tmp_block = last_block; tmp_block != BLOCK_INVALID; tmp_block = hdr.prev){
		sffs_debug(DEBUG_LEVEL, "this block is %d (prev)\n", tmp_block);
		if ( sffs_dev_read(cfg, get_hdr_addr(cfg, tmp_block), &hdr, sizeof(hdr)) != sizeof(hdr) ){
			sffs_error("failed to read\n");
			return -1;
		}

		total++;
		sffs_debug(DEBUG_LEVEL, "set list block %d status to 0x%X\n", tmp_block, status);
		if ( sffs_block_setstatus(cfg, tmp_block, status) < 0 ){
			sffs_error("failed to set status of %d to %d\n", tmp_block, status);
			return -1;
		}
		j++;
	}

	return 0;
}

int sffs_list_discard(const void * cfg, block_t list_block){
	return sffs_list_setblockstatus(cfg, list_block, BLOCK_STATUS_DIRTY);
}

int sffs_list_close(const void * cfg, block_t list_block){
	return sffs_list_setblockstatus(cfg, list_block, BLOCK_STATUS_CLOSED);
}


int sffs_list_append(const void * cfg, sffs_list_t * list, uint8_t type, void * item, int * addr){
	int dev_addr;
	block_t next_block;
	int ret;
	char tmp[list->item_size];

	sffs_debug(DEBUG_LEVEL, "appending to list %d\n", list->current_block);
	while( (ret = sffs_list_getnext(cfg, list, tmp, NULL)) == 0 ){
		//check to see if item is already in the list
		sffs_debug(DEBUG_LEVEL + 4, "scanning list\n");
		if ( memcmp(item, tmp, list->item_size) == 0 ){
			sffs_debug(DEBUG_LEVEL, "Item exists\n");
			return 0; //item already exist
		}
	}

	if ( ret == 1 ){
		dev_addr = get_item_addr(cfg, list->current_block, list->current_item, list->item_size);
	} else {
		next_block = list_add_block(cfg, list->block_data.hdr.serialno, type, list->current_block);
		sffs_debug(DEBUG_LEVEL, "link block to block %d->%d\n", list->current_block, next_block);
		//mcu_debug_user_printf("link block to block %d->%d\n", list->current_block, next_block);
		dev_addr = get_item_addr(cfg, next_block, 0, list->item_size);
	}

	if ( addr != NULL ){
		*addr = dev_addr;
	}

	//write the new entry to disk at dev_addr
	sffs_debug(DEBUG_LEVEL, "appending %d bytes at slot 0 (addr:0x%X)\n", list->item_size, dev_addr);
	/*
	sffs_debug(DEBUG_LEVEL, "Item contents:");
	for(i=0; i < list->item_size; i++){
		if ( CL_DEBUG >= DEBUG_LEVEL ){
			printf("0x%X ", ((unsigned char*)item)[i]);
		}
	}
	if ( CL_DEBUG >= DEBUG_LEVEL ){
		printf("\n");
	}
	 */
	if ( sffs_dev_write(cfg, dev_addr, item, list->item_size) !=
			list->item_size ){
		sffs_error("failed to write item\n");
		return -1;
	}
	sffs_debug(DEBUG_LEVEL, "successfully appended %d bytes at slot 0 (addr:0x%X)\n", list->item_size, dev_addr);

	return 0;
}

block_t sffs_list_consolidate(const void * cfg,
		serial_t serialno,
		block_t list_block,
		uint8_t type,
		int item_size,
		int (*is_dirty)(void*),
		int (*is_free)(void*)){

	sffs_block_data_t sffs_block_data;
	sffs_block_data_t new_sffs_block_data;

	sffs_list_block_t * list;
	sffs_list_block_t * new_list;
	block_t new_block;
	block_t prev_block;
	block_t old_prev_block;
	block_t old_current_block;
	int i, j;
	block_t new_current_block;
	int total_items;

	int copied_items;

	new_block = sffs_block_alloc(cfg, serialno, list_block, type);
	if ( new_block == BLOCK_INVALID ){
		sffs_error("no more blocks\n");
		return BLOCK_INVALID;
	}

	list = (sffs_list_block_t*)(sffs_block_data.data);
	new_list = (sffs_list_block_t*)(new_sffs_block_data.data);
	total_items = calc_total_items(cfg, item_size);

	sffs_debug(DEBUG_LEVEL, "consolidate list on new block %d\n", new_block);

	new_sffs_block_data.hdr.serialno = serialno;
	new_sffs_block_data.hdr.type = type;
	new_sffs_block_data.hdr.status = 0xFF;
	memset(new_sffs_block_data.data, 0xFF, SFFS_LIST_DATA_SIZE);
	new_current_block = new_block;
	new_list->hdr.prev = BLOCK_INVALID;
	old_prev_block = BLOCK_INVALID;

	j = 0;
	copied_items = 0;
	for( list->hdr.next = list_block; list->hdr.next != BLOCK_INVALID ; ){
		old_current_block = list->hdr.next;
		if ( sffs_block_load(cfg, list->hdr.next, &sffs_block_data) < 0 ){
			return -1;
		}

		//make sure the loaded block is valid
		if ( (list->hdr.prev != old_prev_block) || //linking is not correct
				(list_typeisvalid(sffs_block_data.hdr.type) == false) || //block type is not correct
				(serialno != sffs_block_data.hdr.serialno) || //serialno is not correct
				(sffs_block_data.hdr.status != BLOCK_STATUS_CLOSED) //block status must be closed
		){
			sffs_debug(DEBUG_LEVEL, "--------------------END OF LIST %d != %d %d != %d 0x%X != 0x%X\n",
					list->hdr.prev, old_prev_block,
					serialno, sffs_block_data.hdr.serialno,
					sffs_block_data.hdr.status, BLOCK_STATUS_CLOSED);
			break; //end of the list
		}

		old_prev_block = old_current_block;


		for(i = 0; i < total_items; i++){
			if ( is_free( get_item(list, i, item_size) )){
				break;
			} else if ( !is_dirty( get_item(list, i, item_size) ) ){
				memcpy(get_item(new_list, j, item_size), get_item(list, i, item_size), item_size);
				j++;
				copied_items++;
				if ( j == total_items ){
					//save the current block to the disk
					new_list->hdr.next = sffs_block_alloc(cfg, serialno, new_current_block, type | BLOCK_TYPE_LINKED_LIST_FLAG);
					sffs_debug(DEBUG_LEVEL, "save block %d (prev:%d, next:%d)\n", new_current_block, new_list->hdr.prev, new_list->hdr.next);
					//mcu_debug_user_printf("save block %d (prev:%d, next:%d)\n", new_current_block, new_list->hdr.prev, new_list->hdr.next);
					if ( sffs_block_save(cfg, new_current_block, &new_sffs_block_data) < 0 ){
						sffs_error("failed to save block %d\n", new_current_block);
						return -1;
					}
					prev_block = new_current_block;
					new_current_block = new_list->hdr.next;
					memset(new_sffs_block_data.data, 0xFF, SFFS_LIST_DATA_SIZE);
					new_list->hdr.prev = prev_block;
					j = 0;
				}
			}
		}
	}

	//save the last block
	new_list->hdr.next = BLOCK_INVALID;
	if ( sffs_block_save(cfg, new_current_block, &new_sffs_block_data) < 0 ){
		return -1;
	}
	return new_block;
}
