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


#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include "sos/fs/sysfs.h"

#include "mcu/mcu.h"
#include "sos/fs/devfs.h"
#include "../unistd/unistd_fs.h"
#include "../scheduler/scheduler_local.h"
#include "../unistd/unistd_local.h"

static int get_entries(const sysfs_t * list){
	int i;
	int count;
	i = 0;
	count = 0;

	while( sysfs_isterminator(&list[i]) == false ){
		if( list[i].ismounted( list[i].config ) ){
			count++;
		}
		i++;
	}
	return count-1; //subtract one to disregard the root entry
}

int rootfs_init(const void * cfg){
	//no initialization required
	return 0;
}

int rootfs_stat(const void * cfg, const char * path, struct stat * st){
	char buffer[PATH_MAX];
	const sysfs_t * fs;
	int is_root = 0;
	if( path[0] == 0 ){
		is_root = 1;
	}
	strcpy(buffer, "/");
	strcat(buffer, path);
	//find the mount point of the path
	fs = sysfs_find(buffer, false);
	if ( fs == NULL ){
        return SYSFS_SET_RETURN(ENOENT);
	}

	//If the fs is not root, but the mount path says root then the path doesn't exist
	if( (is_root == 0) ){
		if( (fs->mount_path[1] == 0) ){
            return SYSFS_SET_RETURN(ENOENT);
		}
	}

	memset(st, 0, sizeof(struct stat));
	st->st_mode = S_IFDIR | fs->permissions;
	return 0;
}

int rootfs_opendir(const void* cfg, void ** handle, const char * path){
	//if path is not "/", then there is an error
	if ( strcmp(path, "") != 0 ){
        return SYSFS_SET_RETURN(ENOENT);
	}
	//assign the handle value
	*handle = NULL;
	return 0;
}

int rootfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry){
	int total;
	const sysfs_t * list;
	list = (const sysfs_t*)cfg;
	//this loads the loc mounted filesystem
	int i, j;

	//count the total entries
	total = get_entries(list);
	if ( loc < total ){
		i = 0;
		j = 0;
		while( j < loc ){
			if( list[i].ismounted( list[i].config) ){
				j++;
			}
			i++;
		}

		entry->d_ino = loc;
		strcpy(entry->d_name, &(list[loc].mount_path[1]));
		return loc;
	}

	//don't set errno (this just means we are past the end of the list)
	return -1;
}

int rootfs_closedir(const void* cfg, void ** handle){
	//this doesn't need to do anything at all
	return 0;
}

