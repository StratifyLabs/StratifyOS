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




#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "sos/fs/sysfs.h"

#include "sffs_dir.h"
#include <sys/sffs/sffs_list.h>
#include "sffs_serialno.h"

#define DEBUG_LEVEL 10
#define PROB_FAILURE 0.0

int sffs_dir_lookup(const void * cfg, const char * path, sffs_dir_lookup_t * dest, int amode){
	//just go through the serial number list and find the name

	sffs_block_data_t hdr_sffs_block_data;
	cl_hdr_t * hdr;
	sffs_list_t sn_list;
	cl_snlist_item_t item;

	sffs_debug(DEBUG_LEVEL, "initialize list\n");
	if ( cl_snlist_init(cfg, &sn_list, sffs_serialno_getlistblock(cfg) ) < 0 ){
		return -1;
	}

	dest->serialno = SERIALNO_INVALID;
	hdr = (cl_hdr_t *)hdr_sffs_block_data.data;
	while( cl_snlist_getnext(cfg, &sn_list, &item) == 0 ){

		if ( item.serialno == CL_SERIALNO_LIST ){
			continue;
		}

        //the file must be closed (as in, it is in a finalized state to properly exist)
		if ( item.status == SFFS_SNLIST_ITEM_STATUS_CLOSED ){

			if ( sffs_block_load(cfg, item.block, &hdr_sffs_block_data) ){
				sffs_error("failed to load block %d for serialno:%d\n", item.block, item.serialno);
				return -1;
			}

			sffs_debug(DEBUG_LEVEL, "Checking %s to %s\n", path, hdr->open.name);
			if ( strncmp(path, hdr->open.name, NAME_MAX) == 0 ){
				dest->serialno = item.serialno;
				return 0;
			}
		}
	}

	return 0;
}


/*! \details This functions checks to see if a file and/or it's parent directory
 * exists.
 *
 * \return
 * - 0: neither exist
 * - 1: parent only exists
 * - 2: both file and parent exist
 */
int sffs_dir_exists(const void * cfg, const char * path, sffs_dir_lookup_t * dest, int amode){

	sffs_debug(DEBUG_LEVEL, "lookup %s\n", path);
	if ( sffs_dir_lookup(cfg, path, dest, amode) < 0 ){
		sffs_error("failed to lookup\n");
		return -1;
	}

	if ( dest->serialno != SERIALNO_INVALID ){
		//parent and target exist
		return SFFS_DIR_PATH_EXISTS;
	}

	return SFFS_DIR_PARENT_EXISTS; //parent only exists
}



