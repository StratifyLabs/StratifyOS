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



#ifndef SFFS_FILELIST_H_
#define SFFS_FILELIST_H_

#include "../sffs/sffs_block.h"
#include "sffs_list.h"

#define SFFS_FILELIST_BLOCK_ISFREE 0xFFFF
#define SFFS_FILELIST_BLOCK_ISDIRTY 0x0000

/*
 * The file list is a list of blocks that are in a file.
 *
 * The entry includes the block number (physical data location) where the file
 * data is stored and the segment number (data location within the file).
 *
 * Segments are of file and blocks are of the physical drive.
 *
 * Whenever a file is modified, the file's list gets modified.
 *
 *
 *
 */

enum {
	SFFS_FILELIST_STATUS_FREE = 0xFF,
	SFFS_FILELIST_STATUS_CURRENT = 0xFE,
	SFFS_FILELIST_STATUS_OBSOLETE = 0xFC,
	SFFS_FILELIST_STATUS_DIRTY = 0x00
};

/*! \details This structure defines an entry in the file list.  The file
 * list keeps tracks of which blocks in the filesystem correspond to which
 * segment in the file.
 */
typedef struct {
	uint8_t status /*! the status of the entry */;
	uint8_t resd;
	block_t block /*! file block number */;
	serial_t segment /*! the segment number in the file */;
} sffs_filelist_item_t;

block_t sffs_filelist_get(const void * cfg, block_t list_block, int segment, uint8_t status, int * addr);
int sffs_filelist_update(const void * cfg, block_t list_block, int segment, block_t new_block);
int sffs_filelist_setstatus(const void * cfg, uint8_t status, int addr);
block_t sffs_filelist_consolidate(const void * cfg, serial_t serialno, block_t list_block);

int sffs_filelist_makeobsolete(const void * cfg, block_t list_block);

int sffs_filelist_isfree(void * data);

static inline int sffs_filelist_init(const void * cfg, sffs_list_t * list, block_t list_block){
	return sffs_list_init(cfg, list, list_block, sizeof(sffs_filelist_item_t), sffs_filelist_isfree);
}

static inline int sffs_filelist_getnext(const void * cfg, sffs_list_t * list,  sffs_filelist_item_t * item){
	return sffs_list_getnext(cfg, list, item, NULL);
}


#endif /* SFFS_FILELIST_H_ */
