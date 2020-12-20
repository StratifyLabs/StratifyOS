// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md





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



