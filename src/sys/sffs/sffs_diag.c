/* Copyright 2011-2016 Tyler Gilbert; 
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




#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sffs/sffs_diag.h>
#include "sffs_dir.h"
#include "sffs_filelist.h"
#include <sys/sffs/sffs_list.h>
#include "sffs_local.h"
#include "sffs_serialno.h"
#include <unistd.h>


void sffs_diag_show(sffs_diag_t * data){
	printf("Used Blocks: %d (SN%d DSN%d DL%d FL%d FD%d)\n",
			data->used_blocks,
			data->serialno_list_blocks,
			data->del_serialno_list_blocks,
			data->dir_list_blocks,
			data->file_list_blocks,
			data->file_data_blocks);
	printf("Open Blocks: %d\n", data->allocated_blocks);
	printf("Free Blocks: %d\n", data->free_blocks);
	printf("Dirty Blocks: %d\n", data->dirty_blocks);
	printf("Eraseable Blocks: %d\n", data->eraseable_blocks);
	printf("Total Blocks: %d\n", data->total_blocks);
}

void sffs_diag_showfile(sffs_diag_file_t * data){
	printf("Used File Blocks: %d\n", data->used_blocks);
	printf("Allocated File Blocks: %d\n", data->allocated_blocks);
	printf("Dirty File Blocks: %d\n", data->dirty_blocks);
	printf("Total File Blocks: %d\n", data->total_blocks);
}

int sffs_diag_show_eraseable(const void * cfg){
	int i;
	int j;
	sffs_block_data_t sffs_block_data;
	serial_t serials[ sffs_block_geteraseable(cfg) ];
	int eraseable;
	int total_eraseable;
	int uneraseable;

	total_eraseable = 0;
	for(i=0; i < sffs_block_gettotal(cfg); i+= sffs_block_geteraseable(cfg)){

		printf("Blocks %d to %d:  ", i, i+sffs_block_geteraseable(cfg)-1);
		eraseable = 1;
		uneraseable = 0;
		memset(serials, 0, sizeof(serial_t) * sffs_block_geteraseable(cfg) );
		for(j=0; j < sffs_block_geteraseable(cfg); j++){

			if ( i+j != BLOCK_INVALID ){
				if ( sffs_block_load(cfg, i+j, &sffs_block_data) < 0 ){
					printf("failed to load block %d\n", i+j);
					return -1;
				}

				if (sffs_block_data.hdr.status == BLOCK_STATUS_CLOSED) {
					eraseable = 0;
					uneraseable++;
					printf("W%d ", (int)sffs_block_data.hdr.serialno);
				} else if (sffs_block_data.hdr.status == BLOCK_STATUS_OPEN){
					eraseable = 0;
					uneraseable++;
					printf("%XA%d ", sffs_block_data.hdr.type, (int)sffs_block_data.hdr.serialno);
				} else if ( sffs_block_data.hdr.status == BLOCK_STATUS_FREE ){
					eraseable = 0;
					printf("FXX ");
				} else if ( sffs_block_data.hdr.status == BLOCK_STATUS_DISCARDING ){
					eraseable = 0;
					printf("D%d ", (int)sffs_block_data.hdr.serialno);
				} else if ( sffs_block_data.hdr.status == BLOCK_STATUS_DIRTY ){
					printf("X%d ", (int)sffs_block_data.hdr.serialno);
				} else {
					printf("?0x%X ", sffs_block_data.hdr.status);
				}
			}

		}
		if ( eraseable == 1 ){
			printf("Eraseable ");
			total_eraseable+=sffs_block_geteraseable(cfg);
		}
		printf("In Use: %d", uneraseable);
		printf("\n");
	}

	printf("Total Eraseable: %d\n", total_eraseable);

	return 0;

}


int sffs_diag_scan(const void * cfg){
	sffs_block_data_t hdr_sffs_block_data;
	cl_hdr_t * hdr;
	sffs_list_t sn_list;
	cl_snlist_item_t item;
	int ret;
	int num_files;
	int num_dirs;
	int file_size;

	sffs_debug(1, "initialize list\n");
	if ( cl_snlist_init(cfg, &sn_list, sffs_serialno_getlistblock(cfg) ) < 0 ){
		return -1;
	}

	hdr = (cl_hdr_t*)(hdr_sffs_block_data.data);

	file_size = 0;
	num_files = 0;
	num_dirs = 0;
	ret = 0;
	sffs_debug(1, "go through serialno list\n");
	while( cl_snlist_getnext(cfg, &sn_list, &item) == 0 ){

		if ( item.status != SFFS_SNLIST_ITEM_STATUS_CLOSED ){
			//has been deleted
			continue;
		}

		if ( sffs_block_load(cfg, item.block, &hdr_sffs_block_data) < 0 ){
			printf("failed to load block %d\n", item.block);
			return -1;
		}

		if ( hdr_sffs_block_data.hdr.type == BLOCK_TYPE_FILE_HDR ){
			num_files++;
			file_size += hdr->close.size;
			if ( sffs_diag_scanfile(cfg, item.serialno) ){
				sffs_error("Error with file %d\n", item.serialno);
				ret = -1;
			}
		}

	}

	printf("Total Bytes in files: %d\n", file_size);
	printf("Total Files: %d\n", num_files);
	printf("Total Dirs:  %d\n", num_dirs);

	return ret;
}


int sffs_diag_scanfile(const void * cfg, serial_t serialno){
	int ret;
	cl_hdr_t * hdr;
	block_t hdr_block;
	sffs_block_data_t hdr_sffs_block_data;
	sffs_block_data_t sffs_block_data;
	sffs_list_t file_data_list;
	sffs_filelist_item_t list_item;

	if ( serialno == CL_SERIALNO_LIST ){
		return 0;
	}

	ret = 0;
	hdr = (cl_hdr_t*)hdr_sffs_block_data.data;

	//first lookup the file's block number

	hdr_block = sffs_serialno_get(cfg, serialno, SFFS_SNLIST_ITEM_STATUS_CLOSED, NULL);

	if ( sffs_block_load(cfg, hdr_block, &hdr_sffs_block_data) < 0 ){
		printf("failed to load block %d\n", hdr_block);
		return -1;
	}

	if( hdr_sffs_block_data.hdr.type != BLOCK_TYPE_FILE_HDR ){
		printf("Error header is wrong type (0x%X != 0x%X)\n", hdr_sffs_block_data.hdr.type, BLOCK_TYPE_FILE_HDR);
		ret = -1;
	}

	if ( hdr_sffs_block_data.hdr.serialno != serialno ){
		printf("Error file header is wrong serialno (%d != %d)\n", (int)sffs_block_data.hdr.serialno, (int)serialno );
		ret = -1;
	}

	if( sffs_filelist_init(cfg, &file_data_list, hdr->open.content_block) < 0 ){
		printf("Failed to initialize file data list\n");
		return -1;
	}

	if( file_data_list.block_data.hdr.type != BLOCK_TYPE_FILE_LIST ){
		printf("Error file list is wrong type (0x%X != 0x%X)\n", hdr_sffs_block_data.hdr.type, BLOCK_TYPE_FILE_HDR);
		ret = -1;
	}

	if ( file_data_list.block_data.hdr.serialno != serialno ){
		printf("Error file list is wrong serialno (%d != %d)\n", (int)sffs_block_data.hdr.serialno, (int)serialno );
		ret = -1;
	}


	while( sffs_filelist_getnext(cfg, &file_data_list, &list_item) == 0 ){

		if ( list_item.status == SFFS_FILELIST_STATUS_CURRENT ){ //if status is 0, the entry has been discarded

			if ( sffs_block_load(cfg, list_item.block, &sffs_block_data) < 0 ){
				printf("failed to load block %d\n", list_item.block);
				return -1;
			}

			if( sffs_block_data.hdr.type != BLOCK_TYPE_FILE_DATA ){
				printf("Error on segment %d invalid block data type 0x%X\n", (int)list_item.segment, (unsigned int)sffs_block_data.hdr.type);
				ret = -1;
			}

			if ( sffs_block_data.hdr.serialno != serialno ){
				printf("Error file data is wrong serialno (%d != %d)\n", (int)sffs_block_data.hdr.serialno, (int)serialno );
				ret = -1;
			}
		}
	}

	return ret;
}


/*! \details This function gets the diagnostic data structure from
 * the file system.
 */
int sffs_diag_get(const void * cfg, sffs_diag_t * dest){
	int i;
	int j;
	int addr;
	int size;
	int erase_size;
	int eraseable;
	sffs_block_hdr_t hdr;
	int free_blocks;
	int dirty_blocks;
	int written_blocks;

	memset(dest, 0, sizeof(sffs_diag_t));
	erase_size = sffs_dev_geterasesize(cfg);
	size = sffs_dev_getsize(cfg) - erase_size*2;

	for(j=0*BLOCK_SIZE; j < size; j+=erase_size){
		eraseable = 1;

		free_blocks = 0;
		dirty_blocks = 0;
		written_blocks = 0;

		for(i=0; i < erase_size; i+= BLOCK_SIZE){
			addr = j + i;
			if ( sffs_dev_read(cfg, addr, &hdr, sizeof(sffs_block_hdr_t)) != sizeof(sffs_block_hdr_t) ){
				return -1;
			}
			dest->total_blocks++;
			switch(hdr.status){
			case BLOCK_STATUS_FREE:
				dest->free_blocks++;
				free_blocks++;
				if( (i+j) != 0 ){
					eraseable = 0;
				}
				break;
			case BLOCK_STATUS_OPEN:
				dest->allocated_blocks++;
				break;
			case BLOCK_STATUS_CLOSED:
				dest->used_blocks++;
				written_blocks++;
				eraseable = 0;
				if ( 1 ){
					switch(hdr.type & ~BLOCK_TYPE_LINKED_LIST_FLAG){
					case BLOCK_TYPE_SERIALNO_LIST:
						dest->serialno_list_blocks++;
						break;
					case BLOCK_TYPE_SERIALNO_DEL_LIST:
						dest->del_serialno_list_blocks++;
						break;
					case BLOCK_TYPE_DIR_LIST:
						dest->dir_list_blocks++;
						break;
					case BLOCK_TYPE_FILE_LIST:
						dest->file_list_blocks++;
						break;
					case BLOCK_TYPE_FILE_DATA:
						dest->file_data_blocks++;
						break;
					}
				}

				break;
			case BLOCK_STATUS_DIRTY:
				dest->dirty_blocks++;
				dirty_blocks++;
				break;
			default:
				//printf("Unknown block status at 0x%X (0x%X)\n", addr, hdr.status);
				break;
			}
		}

		//printf("%d to %d: dirty: %d free: %d written: %d\n", j/BLOCK_SIZE, (j+erase_size)/BLOCK_SIZE-1, dirty_blocks, free_blocks, written_blocks);

		if ( eraseable == 1 ){
			dest->eraseable_blocks+=blocks_per_eraseable;
		}
	}

	return 0;
}

int sffs_diag_getfile(const void * cfg, const char * path, sffs_diag_file_t * dest){
	int i;
	int j;
	int addr;
	int size;
	int erase_size;
	int blocks_per_eraseable;
	sffs_block_hdr_t hdr;
	serial_t serialno;
	sffs_dir_lookup_t entry;
	int err;

	err = sffs_dir_exists(cfg, path, &entry, R_OK);
	if ( err < 0 ){
		printf("exists failed\n");
		return -1;
	}

	if ( err != SFFS_DIR_PATH_EXISTS ){
		return -1;
	}

	serialno = entry.serialno;

	if ( sffs_diag_scanfile(cfg, entry.serialno) < 0 ){
		printf("file is bad\n");
		exit(5);
	}

	memset(dest, 0, sizeof(sffs_diag_t));
	size = sffs_dev_getsize(cfg);
	erase_size = sffs_dev_geterasesize(cfg);
	blocks_per_eraseable = erase_size / BLOCK_SIZE;

	for(j=0; j < size; j+=erase_size){

		for(i=0; i < erase_size; i+= BLOCK_SIZE){
			addr = j + i;
			if ( sffs_dev_read(cfg, addr, &hdr, sizeof(sffs_block_hdr_t)) != sizeof(sffs_block_hdr_t) ){
				return -1;
			}

			if ( hdr.serialno == serialno ){
				dest->total_blocks++;
				switch(hdr.status){
				case BLOCK_STATUS_OPEN:
					dest->allocated_blocks++;
					break;
				case BLOCK_STATUS_CLOSED:
					dest->used_blocks++;
					break;
				case BLOCK_STATUS_DIRTY:
					dest->dirty_blocks++;
					break;
				default:
					//printf("Invalid block status at 0x%X (0x%X)\n", addr, hdr.status);
					break;
				}
			}
		}
	}

	return 0;
}
