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



#ifndef SFFS_SERIALNO_H_
#define SFFS_SERIALNO_H_

#include "sffs_block.h"
#include "sffs_list.h"
#include "sffs_local.h"

/*
 * Every file is assigned a serial number. The serial number
 * is then used to mark the owner of each block in the block
 * header.
 *
 * At any given time, a serial number has a state. If
 * there is a power failure, the initialization routine
 * can traverse the list of serial numbers to see if any clean-up
 * operations are required.
 *
 *
 *
 *
 */

enum {
	SFFS_SNLIST_ITEM_STATUS_FREE = 0xFF,
	SFFS_SNLIST_ITEM_STATUS_OPEN = 0xFE,
	SFFS_SNLIST_ITEM_STATUS_CLOSING = 0xFC,
	SFFS_SNLIST_ITEM_STATUS_CLOSED = 0xF8,
	SFFS_SNLIST_ITEM_STATUS_DISCARDING_HDR = 0xF0,
	SFFS_SNLIST_ITEM_STATUS_DISCARDING_HDR_LIST = 0xE0,
	SFFS_SNLIST_ITEM_STATUS_DISCARDING = 0xC0,
	SFFS_SNLIST_ITEM_STATUS_DIRTY = 0x00
};


typedef struct MCU_PACK {
	uint8_t status;
	serial_t serialno;
	block_t block;
	uint8_t checksum;
} cl_snlist_item_t;

#define CL_BLOCK_LIST 0x1234

int sffs_serialno_init(const void * cfg, cl_snlist_item_t * bad_serialno_item);
serial_t sffs_serialno_new(const void * cfg); //gets a new number (does not write the list)
block_t sffs_serialno_get(const void * cfg, serial_t serialno, uint8_t status, int * addr);
int sffs_serialno_append(const void * cfg, serial_t serialno, block_t new_block, int * addr, int status); //appends an entry as "open"
int sffs_serialno_setstatus(const void * cfg, int addr, uint8_t status);
int sffs_serialno_consolidate(const void * cfg);
int sffs_serialno_mkfs(const void * cfg);
block_t sffs_serialno_getlistblock(const void * cfg);
int sffs_serialno_isfree(void * data);
int sffs_serialno_scan(serial_t * serialno);

static inline int cl_snlist_init(const void * cfg, sffs_list_t * list, block_t list_block){
	return sffs_list_init(cfg, list, list_block, sizeof(cl_snlist_item_t), sffs_serialno_isfree);
}

static inline int cl_snlist_getnext(const void * cfg, sffs_list_t * status,  cl_snlist_item_t * item){
	return sffs_list_getnext(cfg, status, item, NULL);
}


#endif /* SFFS_SERIALNO_H_ */
