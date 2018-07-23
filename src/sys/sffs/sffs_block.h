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


#ifndef SFFS_BLOCK_H_
#define SFFS_BLOCK_H_

#include "sffs_local.h"

/*
 * Blocks refer to areas of flash. An eraseable block represents a block of
 * flash memory that can be erased. Otherwise, a block is defined by BLOCK_SIZE
 * in sffs_local and represents the smallest writeable portion to disk.
 *
 * This module manages blocks.
 *
 * Blocks can store list data or file data.
 *
 * This is currently built for wear-leveling. There needs to be an option
 * to turn off wear leveling in order to use SFFS with an SD card.
 *
 */


block_t sffs_block_alloc(const void * cfg, serial_t serialno, block_t hint, uint8_t type);
int sffs_block_save(const void * cfg, block_t sffs_block_num, sffs_block_data_t * data);
int sffs_block_saveraw(const void * cfg, block_t sffs_block_num, sffs_block_data_t * data);
int sffs_block_load(const void * cfg, block_t sffs_block_num, sffs_block_data_t * data);
int sffs_block_loadhdr(const void * cfg, sffs_block_hdr_t * dest, block_t src);
int sffs_block_setstatus(const void * cfg, block_t sffs_block_num, uint8_t status);

int sffs_block_discardopen(const void * cfg);

serial_t sffs_block_get_serialno(const void * cfg, block_t block);

block_t sffs_block_geteraseable(const void * cfg);
int sffs_block_gettotal(const void * cfg);
block_t sffs_block_getfirst(const void * cfg);

static inline int sffs_block_discard(const void * cfg, block_t sffs_block_num) MCU_ALWAYS_INLINE;
int sffs_block_discard(const void * cfg, block_t sffs_block_num){
	return sffs_block_setstatus(cfg, sffs_block_num, BLOCK_STATUS_DIRTY);
}

static inline int sffs_block_close(const void * cfg, block_t sffs_block_num) MCU_ALWAYS_INLINE;
int sffs_block_close(const void * cfg, block_t sffs_block_num){
	return sffs_block_setstatus(cfg, sffs_block_num, BLOCK_STATUS_CLOSED);
}

#endif /* SFFS_BLOCK_H_ */
