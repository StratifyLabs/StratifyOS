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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sffs_block.h"
#include <sys/sffs/sffs_dev.h>
#include <sys/sffs/sffs_scratch.h>
#include <sys/sffs/sffs_tp.h>


#define DEBUG_LEVEL 4

static int get_scratch_entries(const void * cfg){
	return sffs_dev_geterasesize(cfg) / (sizeof(sffs_scratch_entry_t) + BLOCK_SIZE);
}

static int get_scratch_addr(const void * cfg){
	return sffs_block_gettotal(cfg) * BLOCK_SIZE;
}

static int get_scratch_entry_addr(const void * cfg, int n){
	return get_scratch_addr(cfg) + n*sizeof(sffs_scratch_entry_t);
}

static int get_scratch_block_addr(const void * cfg, int n){
	return get_scratch_addr(cfg) + sizeof(sffs_scratch_entry_t) * get_scratch_entries(cfg) + BLOCK_SIZE*n;
}

static int save_scratch_entry(const void * cfg, int n, sffs_scratch_entry_t * entry){
	int addr;
	addr = get_scratch_entry_addr(cfg, n); //get the address of the header entry
	if ( sffs_dev_write(cfg, addr, entry, sizeof(*entry)) != sizeof(*entry) ){
		return -1;
	}
	return 0;
}


static void set_checksum(sffs_scratch_entry_t * entry){
	uint8_t * p;
	int i;
	p = (uint8_t*)entry;
	entry->checksum = 0;
	//checksum of list and block
	for(i=0; i < sizeof(sffs_scratch_entry_t) - 1; i++){
		entry->checksum += ~p[i];
	}
}

static int validate_checksum(sffs_scratch_entry_t * entry){
	uint8_t * p;
	int i;
	p = (uint8_t*)entry;
	uint8_t checksum = 0;
	//checksum of list and block
	for(i=0; i < sizeof(sffs_scratch_entry_t) - 1; i++){
		checksum += ~p[i];
	}
	sffs_debug(DEBUG_LEVEL+4, "checksum 0x%X - 0x%X\n", checksum, entry->checksum);
	return checksum - entry->checksum;
}


static int load_scratch_entry(const void * cfg, int n, sffs_scratch_entry_t * entry){
	int addr;
	addr = get_scratch_entry_addr(cfg, n); //get the address of the header entry
	if ( sffs_dev_read(cfg, addr, entry, sizeof(*entry)) != sizeof(*entry) ){
		return -1;
	}
	return 0;
}

static int strike_scratch_entry(const void * cfg, int n){
	int addr;
	sffs_scratch_entry_t entry;
	memset(&entry, 0, sizeof(entry));
	addr = get_scratch_entry_addr(cfg, n); //get the address of the header entry
	if ( sffs_dev_write(cfg, addr, &entry, sizeof(entry)) != sizeof(entry) ){
		return -1;
	}
	return 0;
}

static int scratch_sffs_block_is_ready(const void * cfg, int n){
	int addr;
	sffs_block_data_t tmp;
	int i;
	int block_size = BLOCK_SIZE;
	addr = get_scratch_block_addr(cfg, n);
	//ensure the target block is all 0xFF -- this is done in case the header table is wrong because of a reset/power failure
	if ( sffs_dev_read(cfg, addr, &tmp, sizeof(sffs_block_data_t)) != sizeof(sffs_block_data_t) ){
		return -1;
	}

	for(i=0; i < block_size; i++){
		if ( ((unsigned char*)(&tmp))[i] != 0xFF ){
			return 0; //not writable
		}
	}
	return 1;
}

static int restore_block(const void * cfg, block_t original_block, int n){
	int addr;
	sffs_block_data_t tmp;

	addr = get_scratch_block_addr(cfg, n);
	//ensure the target block is all 0xFF -- this is done in case the header table is wrong because of a reset/power failure
	if ( sffs_dev_read(cfg, addr, &tmp, sizeof(sffs_block_data_t)) != sizeof(sffs_block_data_t) ){
		sffs_error("could not read scratch block at 0x%X\n", addr);
		return -1;
	}

	sffs_debug(DEBUG_LEVEL, "Restore block %d from scratch 0x%X (%d bytes) sn:%d\n", original_block, addr, (int)sizeof(sffs_block_data_t), tmp.hdr.serialno );


	if ( sffs_block_saveraw(cfg, original_block, &tmp) < 0 ){
		sffs_error("Could not save block %d\n", original_block);
		return -1;
	}

	return 0;

}

static int copy_block_to_scratch(const void * cfg, block_t sffs_block_num, int n){
	int addr;
	sffs_block_data_t tmp;

	addr = get_scratch_block_addr(cfg, n);
	if ( sffs_block_load(cfg, sffs_block_num, &tmp) < 0 ){
		sffs_error("FAILED TO LOAD BLOCK\n");
		return -1;
	}

	if ( sffs_dev_write(cfg, addr, &tmp, sizeof(tmp)) != sizeof(tmp) ){
		sffs_error("FAILED TO SAVE TO SCRATCH\n");
		return -1;
	}

	sffs_debug(DEBUG_LEVEL, "Save block %d in scratch at 0x%X (%d bytes) sn:%d\n", sffs_block_num, addr, (int)(sizeof(tmp)), tmp.hdr.serialno);
	return 0;
}

int sffs_scratch_init(const void * cfg){
	//go through the scratch pad and restore any entries that are pending
	return sffs_scratch_restore(cfg);
}

int sffs_scratch_erase(const void * cfg){
	return sffs_dev_erasesection(cfg, get_scratch_addr(cfg) );
}

int sffs_scratch_capacity(const void * cfg){
	int i;
	int total;
	sffs_scratch_entry_t entry;

	total = get_scratch_entries(cfg);

	for(i=0; i < total; i++){

		if ( load_scratch_entry(cfg, i, &entry) < 0 ){
			sffs_error("Error loading scratch entry\n");
			return -1;
		}

		if ( entry.status == SFFS_SCRATCH_STATUS_UNUSED ){
			break;
		}
	}
	return total - i;
}

int sffs_scratch_saveblock(const void * cfg, block_t sffs_block_num){
	int i;
	int total;
	sffs_scratch_entry_t entry;
	total = get_scratch_entries(cfg);

	//find a free entry in the table
	for(i=0; i < total; i++){

		if ( load_scratch_entry(cfg, i, &entry) < 0 ){
			sffs_error("failed to load entry\n");
			return -1;
		}

		if ( entry.status == SFFS_SCRATCH_STATUS_UNUSED ){
			break;
		}
	}

	if ( i == total ){
		//The scratch pad is full
		sffs_error("The scratch area is full\n");
		return -1;
	}

	//check to see if the scratch block is available
	if ( scratch_sffs_block_is_ready(cfg, i) == 0 ){
		//block is not ready -- strike the entry
		if ( strike_scratch_entry(cfg, i) < 0 ){
			sffs_error("failed to strike entry\n");
			return -1;
		}
		sffs_error("block is not ready\n");
		return -1;
	}


	//copy the data to the scratch pad
	if ( copy_block_to_scratch(cfg, sffs_block_num, i) < 0 ){
		sffs_error("couldn't copy block %d to scratch %d\n", sffs_block_num, i);
		return -1;
	}

	//update the table with the entry
	entry.original_block = sffs_block_num;
	entry.status = SFFS_SCRATCH_STATUS_COPIED;
	set_checksum(&entry);
	if ( save_scratch_entry(cfg, i, &entry) < 0 ){
		return -1;
	}

	return 0;
}


int sffs_scratch_restore(const void * cfg){
	//restore all the valid blocks in the scratch area
	int i;
	sffs_scratch_entry_t entry;

	for(i=0; i < get_scratch_entries(cfg); i++){
		if ( load_scratch_entry(cfg, i, &entry) < 0 ){
			sffs_error("failed to load scratch entry %d\n", i);
			return -1;
		}

		if ( entry.status == SFFS_SCRATCH_STATUS_UNUSED ){
			break;
		}

		if ( entry.status == SFFS_SCRATCH_STATUS_COPIED ){

			if( validate_checksum(&entry) == 0 ){ //only restore blocks with valid checksums
				if ( restore_block(cfg, entry.original_block, i) < 0 ){
					sffs_error("restore failed\n");
					return -1;
				}
			}

			if ( strike_scratch_entry(cfg, i) < 0 ){
				sffs_error("Strike failed\n");
				return -1;
			}

		}
	}

	return 0;
}
