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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"

#include "sos/link.h"
#include "sos/fs/devfs.h"
#include "cortexm/task.h"
#include "mcu/debug.h"

#include <fcntl.h>
#include <errno.h>

#include "sos/fs/sysfs.h"
#include "sos/sos.h"
#include "process_start.h"
#include "../scheduler/scheduler_local.h"
#include "../sysfs/appfs_local.h"

#define PATH_ARG_MAX LINK_PATH_ARG_MAX

int launch(const char * path,
			  char * exec_dest,
			  const char * args,
			  int options,
			  int ram_size,
			  int (*update_progress)(const void *, int, int),
			  const void * update_context,
			  char *const envp[]){

	char exec_path[PATH_ARG_MAX];

	//check if path exists
	if( access(path, R_OK) < 0 ){
		return -1;
	}


	if( args != 0 ){
		if( strnlen(args, PATH_ARG_MAX) > PATH_ARG_MAX - PATH_MAX ){
			errno=ENAMETOOLONG;
			return -1;
		}
	}

	if( options & APPFS_FLAG_IS_REPLACE ){
		//kill the process if it is running (or abort)

		//delete the image and re-install

	}

	//if path is already in app then there is no need to install -- just execute
	if( strncmp(path, "/app/", 5) != 0 ){

		if( install(path, exec_path, options, ram_size, update_progress, update_context) < 0 ){
			return -1;
		}

	} else {
		strncpy(exec_path, path, PATH_ARG_MAX-1);
	}

	if( exec_dest != 0 ){
		strncpy(exec_dest, exec_path, PATH_MAX-1);
	}

	if( args ){
		strncat(exec_path, " ", PATH_ARG_MAX-1);
		strncat(exec_path, args, PATH_ARG_MAX-1);
	}

	return process_start(exec_path, envp, options);
}
