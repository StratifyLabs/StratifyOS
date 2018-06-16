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



#ifndef SFFS_LOCAL_H_
#define SFFS_LOCAL_H_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <stdint.h>

#ifdef __SIM__
#include "../sffs/sim_device.h"
#else
#include "sos/fs/devfs.h"
#include "mcu/core.h"
#endif
#include "sffs_dev.h"

typedef u32 serial_t;
typedef u16 block_t;

//block size of 256 will give better performance but use more ram and be less space efficient
#define BLOCK_SIZE 256

//block size 128 will be more space efficient and use less ram but have worse performance
//#define BLOCK_SIZE 128

#define BLOCK_INVALID (0xFFFF)
#define SERIALNO_INVALID (0xFFFFFFFF)
#define SEGMENT_INVALID (0xFFFF)

#define FIRST_BLOCK 1
#define CL_SERIALNO_LIST (0)


enum {
	BLOCK_STATUS_FREE = 0xFF,
	BLOCK_STATUS_OPEN = 0xFE,
	BLOCK_STATUS_CLOSED = 0xFC,
	BLOCK_STATUS_DISCARDING = 0xF8,
	BLOCK_STATUS_DIRTY = 0x00
};

#define BLOCK_TYPE_LIST_FLAG 0x80
#define BLOCK_TYPE_LINKED_LIST_FLAG 0x40
#define BLOCK_TYPE_LIST_MASK (BLOCK_TYPE_LIST_FLAG|BLOCK_TYPE_LINKED_LIST_FLAG)


enum {
	BLOCK_TYPE_SERIALNO_LIST = 0x00 | BLOCK_TYPE_LIST_FLAG,
	BLOCK_TYPE_SERIALNO_DEL_LIST = 0x01 | BLOCK_TYPE_LIST_FLAG,
	BLOCK_TYPE_DIR_HDR = 0x02,
	BLOCK_TYPE_DIR_LIST = 0x03 | BLOCK_TYPE_LIST_FLAG,
	BLOCK_TYPE_FILE_HDR = 0x04,
	BLOCK_TYPE_FILE_LIST = 0x05 | BLOCK_TYPE_LIST_FLAG,
	BLOCK_TYPE_FILE_DATA = 0x06,
	BLOCK_TYPE_LINK_HDR = 0x07,
	BLOCK_TYPE_SYMLINK_HDR = 0x08
};

typedef struct MCU_PACK {
	serial_t serialno;
	u8 type;
	u8 status;
} sffs_block_hdr_t;


#define BLOCK_HEADER_SIZE (sizeof(sffs_block_hdr_t))


#define BLOCK_DATA_SIZE (BLOCK_SIZE - BLOCK_HEADER_SIZE)

//This defines the physical layer
typedef struct MCU_PACK {
	sffs_block_hdr_t hdr;
	char data[BLOCK_DATA_SIZE];
} sffs_block_data_t;

typedef struct {
	char name[NAME_MAX+1];
	block_t content_block;
} cl_hdr_open_t;

typedef struct {
	int32_t size;
} cl_hdr_close_t;

typedef struct MCU_PACK {
	cl_hdr_open_t open;
	cl_hdr_close_t close;
} cl_hdr_t;


typedef struct MCU_PACK {
	block_t hdr_block /*! the block for the file header */;
	block_t segment_list_block /*! The block containing the file's list of segments */;
	int serialno_addr /*! The address of the serial number entry */;
	devfs_async_t * op /*! a pointer to the current operation */;
	int bytes_left /*! the number of bytes read/written so far */;
	int size /*! The size of the file */;
	uint8_t amode /*! The open mode */;
	uint16_t segment /*! The segment of the file */;
	uint32_t mtime /*! The time of the last modification */;
	sffs_block_data_t segment_data; /*! The RAM buffer for the segment */;
} cl_handle_t;

#ifdef __SIM__
#define CL_DEBUG 3
#define CL_ERROR
#define CL_TEST

#define MCU_ALWAYS_INLINE

#define mcu_debug_user_printf(...) printf(__VA_ARGS__)

#else
#include "mcu/debug.h"
#endif


#if (CL_DEBUG > 0)
#include <stdio.h>
#define sffs_debug(LEVEL, ...) do { if ( LEVEL <= CL_DEBUG){ printf("%s:%d: ", __func__, __LINE__); printf(__VA_ARGS__); }} while(0)
#else
#define sffs_debug(LEVEL, ...)
#endif

#ifdef CL_ERROR
#define sffs_error(...) do { printf("ERROR:%d:%s: ", __LINE__, __func__); printf(__VA_ARGS__); } while(0)
#else
#define sffs_error(...)
#endif

#include "sffs_tp.h"


#endif /* SFFS_LOCAL_H_ */
