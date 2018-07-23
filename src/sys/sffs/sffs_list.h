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



#ifndef SFFS_LIST_H_
#define SFFS_LIST_H_

#include <stdint.h>

#include "../sffs/sffs_block.h"

/*
 *
 * This module implements arbitary lists (list of
 * serial number and lists of files for sffs).
 *
 * The lists utilize blocks and are linked together.
 *
 * Items in a list can be discarded. So from time
 * to time lists may need to be consolidated. The consolidation
 * will make the size of the list much smaller. Every time
 * a file is modified, the file's list gets modified as it
 * keeps track of the state of the file.
 *
 *
 *
 *
 */

typedef struct MCU_PACK {
	sffs_block_data_t block_data;
	block_t current_block;
	int8_t current_item;
	uint8_t total_in_block;
	uint8_t item_size;
	int (*is_free)(void*);
} sffs_list_t;


typedef struct MCU_PACK {
	block_t next;
	block_t prev;
} sffs_list_hdr_t;


#define SFFS_LIST_STATUS_FREE (0xFFFF)
#define SFFS_LIST_STATUS_DIRTY (0x0000)
#define SFFS_LIST_NEXT_INVALID (0xFFFF)

#define SFFS_LIST_DATA_SIZE (BLOCK_DATA_SIZE - sizeof(sffs_list_hdr_t))

typedef struct {
	sffs_list_hdr_t hdr;
	char data[SFFS_LIST_DATA_SIZE];
} sffs_list_block_t;

#define SFFS_LIST_NO_ANALYSIS 0
#define SFFS_LIST_DO_ANALYSIS 1

int sffs_list_init(const void * cfg, sffs_list_t * list, block_t list_block, int item_size, int (*is_free)(void*));
int sffs_list_getnext(const void * cfg, sffs_list_t * list,  void * item, int * addr);
int sffs_list_append(const void * cfg, sffs_list_t * list, uint8_t type, void * item, int * addr);
int sffs_list_discard(const void * cfg, block_t list_block);
int sffs_list_close(const void * cfg, block_t list_block);
block_t sffs_list_consolidate(const void * cfg,
		serial_t serialno,
		block_t list_block,
		uint8_t type,
		int item_size,
		int (*is_dirty)(void*),
		int (*is_free)(void*));


#endif /* SFFS_LIST_H_ */
