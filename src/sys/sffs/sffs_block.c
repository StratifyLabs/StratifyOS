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

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

#include "sffs_block.h"
#include <sys/sffs/sffs_scratch.h>
#include "sffs_serialno.h"

#define DEBUG_LEVEL 10

static int get_sffs_block_addr(const void * cfg, block_t block){
	return BLOCK_SIZE * block;
}

static block_t alloc_block(const void * cfg, serial_t serialno, block_t hint, uint8_t type);
static int erase_dirty_blocks(const void * cfg, int max_written);
static int erase_dirty_block(const void * cfg, block_t sffs_block_num);

block_t sffs_block_geteraseable(const void * cfg){
	return sffs_dev_geterasesize(cfg) / BLOCK_SIZE;
}

int sffs_block_gettotal(const void * cfg){
	return (int)(sffs_dev_getsize(cfg) - sffs_dev_geterasesize(cfg)) / BLOCK_SIZE; //don't include the scratch area
}

static int mark_allocated(const void * cfg, block_t block, serial_t serialno, uint8_t type){
	sffs_block_hdr_t hdr;
	hdr.type = type;
	hdr.serialno = serialno;
	hdr.status = BLOCK_STATUS_OPEN;
	return sffs_dev_write(cfg, get_sffs_block_addr(cfg, block), &hdr, sizeof(hdr));
}

/*! \details This function reads the serial number associated with the block.
 * \return The serial number for the specified block.
 */
serial_t sffs_block_get_serialno(const void * cfg, block_t block){
	sffs_block_hdr_t hdr;
	if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, block), &hdr, sizeof(hdr)) ){
		return -1;
	}
	return hdr.serialno;
}

/*! \details This function allocates a new block of data for the specified serial number.
 * The block is preferably allocated in
 * - 1.  an erasable section of blocks all associated with the same serial number
 * - 2.  a new eraseable block or
 * - 3.  an erasable block shared with other serial numbers
 *
 * \return The index of the new block
 */
block_t sffs_block_alloc(const void * cfg, serial_t serialno, block_t hint, uint8_t type){
	block_t ret;
	int i;
	ret = alloc_block(cfg, serialno, hint, type);
	if ( ret == BLOCK_INVALID ){ //all blocks have been written

		//erasing blocks that are mostly dirty helps to wear the flash evenly
		if ( erase_dirty_blocks(cfg,  sffs_block_geteraseable(cfg) >> 4) < 0 ){
			sffs_error("failed to erase dirty blocks\n");
			return BLOCK_INVALID;
		}

		ret = alloc_block(cfg, serialno, hint, type);
		if ( ret == BLOCK_INVALID ){
			for(i = 3; i >= 1; i--){
				//This first looks for blocks that are mostly dirty
				if ( erase_dirty_blocks(cfg, sffs_block_geteraseable(cfg) >> i) < 0 ){
					sffs_error("failed to erase dirty blocks (%d)\n", i);
					return BLOCK_INVALID;
				}
				ret = alloc_block(cfg, serialno, hint, type);
				if ( ret != BLOCK_INVALID ){
					break;
				}
			}

			if ( ret == BLOCK_INVALID ){
				//This will rearrange blocks that are up to 75% clean
				if ( erase_dirty_blocks(cfg,  (sffs_block_geteraseable(cfg) * 3) >> 2) < 0 ){
					sffs_error("failed to erase dirty blocks here\n");
					return BLOCK_INVALID;
				}
				ret = alloc_block(cfg, serialno, hint, type);
			}
		}
	}
	return ret;
}


/*! \details This function saves the block data pointed to by \a src to the
 * location on disk associated with block \a dest.
 *
 * \return Zero on success
 */
int sffs_block_save(const void * cfg, block_t sffs_block_num, sffs_block_data_t * data){

	if( sffs_block_num == BLOCK_INVALID ){
		sffs_error("invalid block\n");
		return -1;
	}

	sffs_debug(DEBUG_LEVEL + 3, "save block %d\n", sffs_block_num);
	data->hdr.status = BLOCK_STATUS_OPEN; //the block must be closed at a later time using sffs_block_close()
	if ( sffs_dev_write(cfg, get_sffs_block_addr(cfg, sffs_block_num) + offsetof(sffs_block_hdr_t, status),
			&(data->hdr.status),
			sizeof(*data) - offsetof(sffs_block_hdr_t, status)) !=
					sizeof(*data) - offsetof(sffs_block_hdr_t, status) ){
		sffs_error("failed to save block\n");
		return -1;
	}

	return 0;
}

int sffs_block_saveraw(const void * cfg, block_t sffs_block_num, sffs_block_data_t * data){
	if ( sffs_dev_write(cfg, get_sffs_block_addr(cfg, sffs_block_num), data, sizeof(*data) ) != sizeof(*data)  ){
		return -1;
	}
	return 0;
}

/*! \details This function loads the data associated with block \a src to the location
 * pointed to by \a dest.
 *
 * \return Zero on success
 */
int sffs_block_load(const void * cfg, block_t sffs_block_num, sffs_block_data_t * data){
	//load the data from the device
	int block_size = BLOCK_SIZE;
	sffs_debug(DEBUG_LEVEL + 2, "load block %d\n", sffs_block_num);

	if ( sffs_block_num == BLOCK_INVALID ){
		return -1;
	}

	if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, sffs_block_num), data, block_size) != block_size ){
		sffs_error("failed to read\n");
		return -1;
	}
	return 0;
}

int sffs_block_loadhdr(const void * cfg, sffs_block_hdr_t * dest, block_t src){
	//load the data from the device
	sffs_debug(DEBUG_LEVEL, "load block %d\n", src);

	if ( src == BLOCK_INVALID ){
		return -1;
	}

	if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, src), dest, sizeof(sffs_block_hdr_t)) != sizeof(sffs_block_hdr_t) ){
		sffs_error("failed to read\n");
		return -1;
	}
	return 0;
}

int sffs_block_setstatus(const void * cfg, block_t block, uint8_t status){
	if ( block == BLOCK_INVALID ){
		return -1;
	}
	return sffs_dev_write(cfg, get_sffs_block_addr(cfg, block) + offsetof(sffs_block_hdr_t, status), &status, sizeof(status));
}

int sffs_block_discardopen(const void * cfg){
	sffs_block_hdr_t hdr;
	int i;
	int total_blocks;

	total_blocks = sffs_block_gettotal(cfg); //total number of blocks on the device

	for(i = FIRST_BLOCK; i < total_blocks; i++){
		//read the header
		if ( sffs_block_loadhdr(cfg, &hdr, i) < 0 ){
			sffs_error("failed to load header\n");
			return -1;
		}

		//check status for OPEN
		if (hdr.status == BLOCK_STATUS_OPEN){
			//discard any OPEN blocks
			if ( sffs_block_discard(cfg, i) < 0 ){
				sffs_error("failed to discard\n");
				return -1;
			}
		}
	}
	return 0;
}


block_t alloc_block(const void * cfg, serial_t serialno, block_t hint, uint8_t type){
	sffs_block_hdr_t hdr;
	int i;
	int j;
	int first_loop;
	int total_blocks;
	int eraseable_blocks;
	int first;

	eraseable_blocks = sffs_block_geteraseable(cfg);  //number of blocks that are eraseable contiguously
	total_blocks = sffs_block_gettotal(cfg); //total number of blocks on the device
	first = FIRST_BLOCK;

	if ( hint < first ){
		hint = first;
	}

	if ( hint != BLOCK_INVALID ){
		first_loop = eraseable_blocks - ( hint % eraseable_blocks) + hint;

		//starting at hint -- find a free block within the erasable block

		for(i = hint+1; i < first_loop; i++){
			if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, i), &hdr, sizeof(hdr)) != sizeof(hdr) ){
				sffs_error("failed to read device\n");
				return BLOCK_INVALID;
			}

			if ( hdr.status == BLOCK_STATUS_FREE ){
				if ( mark_allocated(cfg, i, serialno, type) < 0 ){
					sffs_error("failed to mark block allocated\n");
					return BLOCK_INVALID;
				}
				return i;
			}
		}

	} else {
		i = first;
	}


	//now try to find a free erasable block
	for( ; i < total_blocks; i += eraseable_blocks){

		for(j = 0; j < eraseable_blocks; j++){

			if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, i+j), &hdr, sizeof(hdr)) != sizeof(hdr) ){
				sffs_error("failed to read device here\n");
				return BLOCK_INVALID;
			}

			//See if this eraseable block is used by another serial number
			if ((hdr.status == BLOCK_STATUS_CLOSED) || (hdr.status == BLOCK_STATUS_OPEN)){
				if ( hdr.serialno != serialno ){
					break;
				} else {
					continue;
				}
			}

			if ( hdr.status == BLOCK_STATUS_FREE ){
				if ( mark_allocated(cfg, i+j, serialno, type) < 0 ){
					sffs_error("failed to mark block allocated here\n");
					return BLOCK_INVALID;
				}
				return (i + j);
			}
		}
	}


	//now just find a block anywhere
	for(i = first; i < total_blocks; i++){
		if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, i), &hdr, sizeof(hdr)) != sizeof(hdr) ){
			sffs_error("failed to read device there\n");
			return BLOCK_INVALID;
		}

		if ( hdr.status == BLOCK_STATUS_FREE ){
			if ( mark_allocated(cfg, i, serialno, type) < 0 ){
				sffs_error("failed to mark block allocated there\n");
				return BLOCK_INVALID;
			}
			return i;
		}
	}

	sffs_debug(DEBUG_LEVEL, "never found a block\n");
	return BLOCK_INVALID;
}

int erase_dirty_blocks(const void * cfg, int max_written){
	int i;
	int j;
	int total_blocks;
	int eraseable_blocks;
	sffs_block_hdr_t hdr;

	int written;
	bool do_erase;

	eraseable_blocks = sffs_block_geteraseable(cfg);  //number of blocks that are eraseable contiguously
	total_blocks = sffs_block_gettotal(cfg); //total number of blocks on the device

	//now try to find a free erasable block
	for(i = 0; i < total_blocks; i += eraseable_blocks){
		written = 0;
		do_erase = true;
		for(j = 0; j < eraseable_blocks; j++){

			if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, i+j), &hdr, sizeof(hdr)) != sizeof(hdr) ){
				return -1;
			}

			//See if this eraseable block is used by another serial number
			if ( hdr.status == BLOCK_STATUS_CLOSED ){
				if ( hdr.serialno == CL_SERIALNO_LIST ){
					do_erase = false;
					break;
				}
				written++; //count how many blocks are finalized
			} else if ( hdr.status == BLOCK_STATUS_OPEN ){
				do_erase = false;
				break;
			} else if ( (hdr.status == BLOCK_STATUS_FREE) && ((i+j)!=0) ){
				do_erase = false;
				break;
			}
		}

		if ( (do_erase == true) && (written < max_written) ){
			if ( written > sffs_scratch_capacity(cfg) ){
				if ( sffs_scratch_erase(cfg) < 0 ){
					sffs_error("failed to erase scratch area\n");
					return -1;
				}
			}

			if ( erase_dirty_block(cfg, i) < 0 ){
				sffs_error("failed to erase dirty blocks\n");
				return -1;
			}
		}

	}
	return 0;
}

int erase_dirty_block(const void * cfg, block_t sffs_block_num){
	//sffs_block_num should be the start of an eraseable block
	int i;
	int eraseable_blocks;
	sffs_block_hdr_t hdr;


	eraseable_blocks = sffs_block_geteraseable(cfg);  //number of blocks that are eraseable contiguously
	for(i = sffs_block_num; i < (sffs_block_num + eraseable_blocks); i++){

		if ( sffs_dev_read(cfg, get_sffs_block_addr(cfg, i), &hdr, sizeof(hdr)) != sizeof(hdr) ){
			sffs_error("failed to read device\n");
			return -1;
		}

		if ( (hdr.status == BLOCK_STATUS_CLOSED) ||
				(hdr.status == BLOCK_STATUS_OPEN) ){
			//Save this block in the scratch area
			if ( sffs_scratch_saveblock(cfg, i) < 0 ){
				sffs_error("failed to save block %d\n", i);
				return -1;
			}
		}
	}

	CL_TP_DESC(CL_PROB_RARE, "saved to scratch");

	//erase the eraseable block
	if ( sffs_dev_erasesection(cfg, get_sffs_block_addr(cfg, sffs_block_num)) < 0 ){
		sffs_error("failed to erase section\n");
		return -1;
	}

	CL_TP_DESC(CL_PROB_RARE, "section erased");

	//restore the scratch area
	if ( sffs_scratch_restore(cfg) < 0 ){
		sffs_error("failed to restore scratch area\n");
		return -1;
	}

	return 0;

}
