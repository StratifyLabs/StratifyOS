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

#include "link_local.h"


//Access to directories
int link_mkdir(link_transport_mdriver_t * driver, const char * path, link_mode_t mode){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	link_debug(LINK_DEBUG_MESSAGE, "mkdir %s 0%o", path, mode);

	op.mkdir.cmd = LINK_CMD_MKDIR;
	op.mkdir.mode = mode;
	op.mkdir.path_size = strlen(path) + 1;

	link_debug(LINK_DEBUG_MESSAGE, "Write op");
	err = link_transport_masterwrite(driver, &op, sizeof(op));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write path");
	len = link_transport_masterwrite(driver, path, op.mkdir.path_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("Failed to get reply");
		return err;
	}

	if( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to mkdir (%d)", link_errno);
	}

	return reply.err;

}

int link_rmdir(link_transport_mdriver_t * driver, const char * path){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	link_debug(LINK_DEBUG_MESSAGE, "rmdir %s", path);

	op.rmdir.cmd = LINK_CMD_RMDIR;
	op.rmdir.path_size = strlen(path) + 1;

	err = link_transport_masterwrite(driver, &op, sizeof(op));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	len = link_transport_masterwrite(driver, path, op.rmdir.path_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to rmdir (%d)", link_errno);
	}

	return reply.err;
}

int link_opendir(link_transport_mdriver_t * driver, const char * dirname){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	if ( driver == NULL ){
		link_error("No device");
		return 0;
	}

	op.opendir.cmd = LINK_CMD_OPENDIR;
	op.opendir.path_size = strlen(dirname) + 1;

	if ( dirname == NULL ){
		link_error("Directory name is NULL");
		return 0;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Write op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_opendir_t));
	if ( err < 0 ){
		link_error("Failed to transfer command");
		return 0;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write path %s", dirname);
	len = link_transport_masterwrite(driver, dirname, op.opendir.path_size);
	if ( len < 0 ){
		link_error("Failed to write bulk out");
		return 0;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Write path len is %d 0x%X", len, reply.err);
	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("Failed to read bulk in");
		return 0;
	}

	if( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to opendir (%d)", link_errno);
	}

	return reply.err;
}

int link_readdir_r(link_transport_mdriver_t * driver, int dirp, struct link_dirent * entry, struct link_dirent ** result){
	link_op_t op;
	link_reply_t reply;
	int len;

	if ( driver == NULL ){ return -1; }
	if ( result != NULL ){ *result = NULL; }

	op.readdir.cmd = LINK_CMD_READDIR;
	op.readdir.dirp = dirp;

	link_debug(LINK_DEBUG_MESSAGE, "Write op");
	if (link_transport_masterwrite(driver, &op, sizeof(link_readdir_t)) < 0){
		return -1;
	}

	reply.err = 0;
	reply.err_number = 0;
	link_debug(LINK_DEBUG_MESSAGE, "Read reply %ld", sizeof(reply));
	if (link_transport_masterread(driver, &reply, sizeof(reply)) < 0){
		return -1;
	}

	if( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to readdir (%d)", link_errno);
		return reply.err;
	}


	//Read the bulk in buffer for the result of the read
	link_debug(LINK_DEBUG_MESSAGE, "Read link dirent");
	len = link_transport_masterread(driver, entry, sizeof(struct link_dirent));
	if ( len < 0 ){
		link_error("Failed to read dirent");
		return -1;
	}

	if ( result != NULL ){
		*result = entry;
	}

	return 0;
}

int link_closedir(link_transport_mdriver_t * driver, int dirp){
	link_op_t op;
	link_reply_t reply;
	int err;

	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	op.closedir.cmd = LINK_CMD_CLOSEDIR;
	op.closedir.dirp = dirp;

	link_debug(LINK_DEBUG_MESSAGE, "Send op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_closedir_t));
	if ( err < 0 ){
		return err;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Get Reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to closedir (%d)", link_errno);
	}
	return reply.err;

}
