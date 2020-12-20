// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



#ifndef SFFS_SCRATCH_H_
#define SFFS_SCRATCH_H_

#include "../sffs/sffs_local.h"


/*
 * The scratch pad is used for consolidating data blocks. Once
 * the filesystem is full, the scratch pad is reserved as an area
 * that can be used to copy data blocks which can be restored once
 * an eraseable block has been erased.
 *
 * For example, if the eraseable block size is 4096 and the block size
 * is 256, there can be up to 16 blocks in an eraseable block. Any
 * entries in the eraseable block that are dirty can be erased. Blocks
 * that are not directly are copied to the scratch pad which the block is erased.
 * The non-dirty data is then restored.
 *
 *
 *
 */


enum {
	SFFS_SCRATCH_STATUS_UNUSED = 0xFF,
	SFFS_SCRATCH_STATUS_ALLOCATED = 0xFE,
	SFFS_SCRATCH_STATUS_COPIED = 0xFC,
	SFFS_SCRATCH_STATUS_ERASED = 0xF8,
	SFFS_SCRATCH_STATUS_RESTORED = 0x00
};

typedef struct MCU_PACK {
	u8 status;
	block_t original_block; //0xFF for unused, X for in use, and 0x00 for dirty
	u8 checksum;
} sffs_scratch_entry_t;


//return the number of free blocks in the scratchpad
int sffs_scratch_init(const void * cfg);
int sffs_scratch_erase(const void * cfg);
int sffs_scratch_capacity(const void * cfg);
int sffs_scratch_saveblock(const void * cfg, block_t sffs_block_num);
int sffs_scratch_restore(const void * cfg);


//on restart -- any entries in the scratch area must be restored to original locations


#endif /* SFFS_SCRATCH_H_ */
