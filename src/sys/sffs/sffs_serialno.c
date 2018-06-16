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



#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <sys/sffs/sffs_list.h>
#include "sffs_serialno.h"

#define DEBUG_LEVEL 10


#define SERIALNO_DEL_AVAILABLE (0x01)
#define SERIALNO_DEL_NOT_AVAILABLE (0)

enum {
	SFFS_INDEX_STATUS_FREE = 0xFF,
	SFFS_INDEX_STATUS_OPEN = 0xFE,
	SFFS_INDEX_STATUS_CLOSING = 0xFC,
	SFFS_INDEX_STATUS_CLOSED = 0xF8,
	SFFS_INDEX_STATUS_DISCARDING = 0xF0,
	SFFS_INDEX_STATUS_DIRTY = 0x00
};

static void set_checksum(cl_snlist_item_t * entry);
static int validate_checksum(cl_snlist_item_t * entry);
static block_t find_list_block(const void * cfg);
static int is_dirty(void * data);
static int consolidate_list(const void * cfg, int (*is_free)(void*), int (*is_dirty)(void*));


void set_checksum(cl_snlist_item_t * entry){
	uint8_t * p;
	int i;
	p = (uint8_t*)entry;
	entry->checksum = 0;
	//checksum of list and block
	for(i=1; i < sizeof(cl_snlist_item_t) - 1; i++){
		entry->checksum += ~p[i];
	}
}

int validate_checksum(cl_snlist_item_t * entry){
	uint8_t * p;
	int i;
	p = (uint8_t*)entry;
	uint8_t checksum = 0;
	//checksum of list and block
	for(i=1; i < sizeof(cl_snlist_item_t) - 1; i++){
		checksum += ~p[i];
	}
	sffs_debug(DEBUG_LEVEL+4, "checksum 0x%X - 0x%X\n", checksum, entry->checksum);
	return checksum - entry->checksum;
}

block_t find_list_block(const void * cfg){
	sffs_block_hdr_t sffs_block_hdr;
	block_t list_block;
	block_t check_block;
	int addr;

	list_block = 0;

	while( sffs_block_loadhdr(cfg, &sffs_block_hdr, list_block) == 0 ){

		if ( ( (sffs_block_hdr.status == BLOCK_STATUS_OPEN) ||
				(sffs_block_hdr.status == BLOCK_STATUS_CLOSED) ) &&
				(sffs_block_hdr.type == BLOCK_TYPE_SERIALNO_LIST) ){
			sffs_dev_setlist_block(cfg, list_block);
			sffs_debug(DEBUG_LEVEL, "list block status is 0x%X\n", sffs_block_hdr.status);
			check_block = sffs_serialno_get(cfg, CL_SERIALNO_LIST, SFFS_SNLIST_ITEM_STATUS_CLOSED, NULL);
			sffs_debug(DEBUG_LEVEL, "check block CLOSED is %d\n", check_block);
			if ( check_block == CL_BLOCK_LIST ){

				if( sffs_block_hdr.status == BLOCK_STATUS_OPEN ){ //make sure the list was properly closed
					if ( sffs_list_close(cfg, list_block) < 0 ){
						sffs_error("failed to close serialno blocks\n");
						return BLOCK_INVALID;
					}
				}

				return list_block;
			} else {

				check_block = sffs_serialno_get(cfg, CL_SERIALNO_LIST, SFFS_SNLIST_ITEM_STATUS_DISCARDING, &addr);
				sffs_debug(DEBUG_LEVEL, "check block DISCARDING is %d\n", check_block);
				if ( check_block == CL_BLOCK_LIST ){
					if ( sffs_list_discard(cfg, list_block) < 0 ){
						sffs_dev_setlist_block(cfg, BLOCK_INVALID);
						sffs_error("failed to discard old list\n");
						return BLOCK_INVALID;
					}

					if ( sffs_serialno_setstatus(cfg, addr, SFFS_SNLIST_ITEM_STATUS_DIRTY) < 0 ){
						sffs_error("failed to mark the list as discarded\n");
						return BLOCK_INVALID;
					}
				}
				sffs_dev_setlist_block(cfg, BLOCK_INVALID);
			}
		}

		list_block++;
	}
	return BLOCK_INVALID;
}

int sffs_serialno_isfree(void * data){
	cl_snlist_item_t * item;
	item = (cl_snlist_item_t*)data;

	if ( item->status == SFFS_SNLIST_ITEM_STATUS_FREE ){
		return 1;
	} else {
		return 0;
	}
}

int is_dirty(void * data){
	cl_snlist_item_t * item;
	item = (cl_snlist_item_t*)data;
	if ( item->status == SFFS_SNLIST_ITEM_STATUS_DIRTY ){
		return 1;
	} else {
		return 0;
	}
}

block_t sffs_serialno_getlistblock(const void * cfg){
	return sffs_dev_getlist_block(cfg);
}


int sffs_serialno_init(const void * cfg, cl_snlist_item_t * bad_serialno_item){
	sffs_list_t list;
	cl_snlist_item_t item;
	serial_t max;
	int ret;
	int addr;
	ret = 0;
	serial_t sn;
	block_t sn_list_block;

	sn_list_block = sffs_dev_getlist_block(cfg);


	if ( (sn_list_block == BLOCK_INVALID) || (sn_list_block == 0) ){
		sn_list_block = find_list_block(cfg);
		if ( sffs_dev_getlist_block(cfg) == BLOCK_INVALID ){
			sffs_error("serial number list not found\n");
			return -1;
		}

		sffs_dev_setlist_block(cfg, sn_list_block);
		//this makes sure the list is closed in case it was consolidating on the last reset
		if ( sffs_serialno_consolidate(cfg) < 0 ){
			sffs_error("failed to consolidate the sn list\n");
			//this will happen if the disk is full -- but allow the program to continue so space can be freed
		}
	}

	//ensure each list is valid
	sffs_debug(DEBUG_LEVEL, "SN list block is %d\n", sn_list_block);
	if ( cl_snlist_init(cfg, &list, sn_list_block) < 0) {
		sffs_error("failed to initialize list\n");
		return -1;
	}
	//assign sn to the highest number in the serial number list
	max = 0;
	sffs_debug(DEBUG_LEVEL, "looking for max\n");
	while( sffs_list_getnext(cfg, &list, &item, &addr) == 0 ){

		if ( item.status == SFFS_SNLIST_ITEM_STATUS_CLOSED ){
			if (item.serialno > max){
				max = item.serialno;
			}
		} else if ( (item.status != SFFS_SNLIST_ITEM_STATUS_FREE) && (item.status != SFFS_SNLIST_ITEM_STATUS_DIRTY) ){
			memcpy(bad_serialno_item, &item, sizeof(item));
			sffs_debug(DEBUG_LEVEL, "Bad serialno %d at %d status:0x%X\n", item.serialno, item.block, item.status);
			return 1;
		}
	}

	if ( max <= 0 ){
		max = 1;
	}
	sn = max + 1;
	if ( sn == SERIALNO_INVALID ){
		sn = CL_SERIALNO_LIST + 1;
		//the device is full
	}

	sffs_dev_setserialno(cfg, sn);

	sffs_debug(DEBUG_LEVEL, "start serialno is %d\n", sn);

	return ret;
}

int sffs_serialno_mkfs(const void * cfg){
	block_t sn_list_block;

	//allocate blocks for the new lists
	if ( (sn_list_block = sffs_block_alloc(cfg, CL_SERIALNO_LIST, BLOCK_INVALID, BLOCK_TYPE_SERIALNO_LIST)) == BLOCK_INVALID ){
		sffs_error("didn't allocate %d\n", sn_list_block);
		return -1;
	}

	if ( sffs_block_close(cfg, sn_list_block) < 0 ){
		sffs_error("failed to close main serialno list\n");
		return -1;
	}

	sffs_debug(DEBUG_LEVEL, "allocated list block at %d\n", sn_list_block);

	sffs_dev_setlist_block(cfg, sn_list_block);

	if ( sffs_serialno_append(cfg, CL_SERIALNO_LIST, CL_BLOCK_LIST, NULL, SFFS_SNLIST_ITEM_STATUS_CLOSED) < 0 ){
		sffs_error("failed to append zero entry\n");
		return -1;
	}

	return 0;
}

int consolidate_list(const void * cfg, int (*is_free)(void*), int (*is_dirty)(void*)){
	int dev_addr;
	block_t tmp_block;
	block_t new_block;
	serial_t sn_list_block;

	sn_list_block = sffs_dev_getlist_block(cfg);

	//consolidate the list using sffs_list_consolidate()
	if ( (new_block = sffs_list_consolidate(cfg,
			CL_SERIALNO_LIST,
			sn_list_block,
			BLOCK_TYPE_SERIALNO_LIST,
			sizeof(cl_snlist_item_t),
			is_dirty,
			sffs_serialno_isfree)) == BLOCK_INVALID ){
		sffs_error("new block is not valid\n");
		return -1;
	}

	CL_TP_DESC(CL_PROB_COMMON, "after consolidate but before close");

	sffs_debug(DEBUG_LEVEL, "get addr to close old list\n");
	if( sffs_serialno_get(cfg, CL_SERIALNO_LIST, SFFS_SNLIST_ITEM_STATUS_CLOSED, &dev_addr) == BLOCK_INVALID ){
		sffs_error("failed to get zero entry\n");
		return -1;
	}

	sffs_debug(DEBUG_LEVEL, "mark old list as discarding\n");
	if ( sffs_serialno_setstatus(cfg, dev_addr, SFFS_SNLIST_ITEM_STATUS_DISCARDING) < 0 ){
		sffs_error("failed to set status to DISCARDING\n");
		return -1;
	}

	tmp_block = sn_list_block;
	sn_list_block = new_block;

	sffs_debug(DEBUG_LEVEL, "close the new list (%d)\n", new_block);
	//now close the list (from front to back) -- on restart it must be checked for closure before cleaning
	if ( sffs_list_close(cfg, new_block) < 0 ){
		sffs_error("failed to close list\n");
		return -1;
	}

	CL_TP_DESC(CL_PROB_COMMON, "after consolidate but before discarding old list");

	sffs_debug(DEBUG_LEVEL, "discard the old list (%d)\n", tmp_block);
	if ( sffs_list_discard(cfg, tmp_block) < 0 ){
		sffs_error("failed to discard old list\n");
		return -1;
	}

	if ( sffs_serialno_setstatus(cfg, dev_addr, SFFS_SNLIST_ITEM_STATUS_DIRTY) < 0 ){
		sffs_error("failed to mark the list as discarded\n");
		return BLOCK_INVALID;
	}

	sffs_dev_setlist_block(cfg, sn_list_block);

	return 0;
}

int sffs_serialno_consolidate(const void * cfg){
	sffs_debug(DEBUG_LEVEL, "consolidate main\n");
	if( consolidate_list(cfg, sffs_serialno_isfree, is_dirty) < 0 ){
		sffs_error("failed to consolidate main serial number list\n");
		return -1;
	}
	return 0;
}


serial_t sffs_serialno_new(const void * cfg){
	sffs_list_t list;
	cl_snlist_item_t item;
	int dev_addr;
	serial_t ret;
	serial_t sn;

	sffs_debug(DEBUG_LEVEL, "list starts on block %d\n", sffs_dev_getlist_block(cfg) );

	sn = sffs_dev_getserialno(cfg); //retrieves the current serial number
	sn++;
	if( sn == SERIALNO_INVALID ){
		//The highest number if 4 billion
		sn = CL_SERIALNO_LIST + 1;
		sffs_error("out of serial numbers\n"); //this should be a no space errno
		return SERIALNO_INVALID;
	}
	sffs_dev_setserialno(cfg, sn);

	return sn;

	if ( cl_snlist_init(cfg, &list, sffs_dev_getlist_block(cfg)) < 0 ){
		sffs_error("failed to init list\n");
		return BLOCK_INVALID;
	}

	if( list.block_data.hdr.status == 0x00 ){
		sffs_error("sn list block (%d) is dirty\n", list.block_data.hdr.status);
		//this needs to be a panic (reset device so that the sn list can re-initialize)
#ifdef __SIM__
		printf("No new serial numbers\n");
		exit(1);
#endif
		return BLOCK_INVALID;
	}

	ret = 1;
	while( sffs_list_getnext(cfg, &list, &item, &dev_addr) == 0 ){

		if ( (item.status != SFFS_SNLIST_ITEM_STATUS_DIRTY) && (item.serialno == sn) ){
			continue;
		} else {
			ret = sn;
			break;
		}
		sn++;
		if( sn == 65535 ){
			sn = CL_SERIALNO_LIST + 1;
		}
	}
	sn++;
	if( sn == 65535 ){
		sn = CL_SERIALNO_LIST + 1;
	}
	sffs_dev_setserialno(cfg, sn);
	sffs_debug(DEBUG_LEVEL, "new serial number is %d\n", ret);
	return ret;
}



block_t sffs_serialno_get(const void * cfg, serial_t serialno, uint8_t status, int * addr){
	sffs_list_t list;
	cl_snlist_item_t item;
	int dev_addr;

	sffs_debug(DEBUG_LEVEL, "list starts on block %d\n", sffs_dev_getlist_block(cfg));

	if ( cl_snlist_init(cfg, &list, sffs_dev_getlist_block(cfg)) < 0 ){
		sffs_error("failed to init list\n");
		return BLOCK_INVALID;
	}
	sffs_debug(DEBUG_LEVEL, "scanning serialno list for %d status 0x%X (0x%X %d 0x%X)\n",
			serialno,
			status,
			list.block_data.hdr.status,
			list.block_data.hdr.serialno,
			list.block_data.hdr.type);

	while( sffs_list_getnext(cfg, &list, &item, &dev_addr) == 0 ){
		sffs_debug(DEBUG_LEVEL+4, "status:0x%X serialno:%d block:%d checksum:%d from addr:0x%X\n",
				item.status,
				item.serialno,
				item.block,
				item.checksum,
				dev_addr);

		if( validate_checksum(&item) == 0 ){
			if ( (item.status == status) && (item.serialno == serialno) ){
				//TODO check the checksum as well
				if ( addr != NULL ){
					*addr = dev_addr;
				}
				return item.block;
			}
		} else if ( item.status != SFFS_SNLIST_ITEM_STATUS_DIRTY ){
			if ( sffs_serialno_setstatus(cfg, dev_addr, SFFS_SNLIST_ITEM_STATUS_DIRTY) < 0 ){
				sffs_error("failed to discard invalid checksum entry\n");
				return -1;
			}
		}
	}

	sffs_debug(DEBUG_LEVEL, "Entry doesn't exist\n");

	return BLOCK_INVALID;
}

int sffs_serialno_setstatus(const void * cfg, int addr, uint8_t status){
	sffs_debug(DEBUG_LEVEL, "writing addr 0x%X\n", addr);
	if ( sffs_dev_write(cfg, addr + offsetof(cl_snlist_item_t, status), &status, sizeof(status)) != sizeof(status) ){
		sffs_error("failed to set status at 0x%X\n", addr);
		return -1;
	}
	return 0;
}

int sffs_serialno_append(const void * cfg, serial_t serialno, block_t new_block, int * addr, int status){
	cl_snlist_item_t item;
	sffs_list_t list;
	//The serial number is at the head of every block
	sffs_debug(DEBUG_LEVEL, "append %d to sn list\n", serialno);
	item.status = status;
	item.serialno = serialno;
	item.block = new_block;
	set_checksum(&item);
	sffs_debug(DEBUG_LEVEL+2, "init sn list from block %d\n", sffs_dev_getlist_block(cfg));
	if ( cl_snlist_init(cfg, &list, sffs_dev_getlist_block(cfg)) <  0 ){
		sffs_error("failed to init list at block %d\n", new_block);
		return -1;
	}
	sffs_debug(DEBUG_LEVEL, "append entry to list 0x%X %d %d 0x%X\n",
			item.status,
			item.serialno,
			item.block,
			item.checksum);
	return sffs_list_append(cfg,
			&list,
			BLOCK_TYPE_SERIALNO_LIST,
			&item,
			addr);
}

