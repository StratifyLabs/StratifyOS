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

/*! \addtogroup SYSFS
 * @{
 */

/*! \file */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "sysfs.h"

const char sysfs_validset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.";


//get the filename from the end of the path
const char * sysfs_getfilename(const char * path, int * elements){
	const char * name;
	int tmp;
	name = path;
	tmp = 1;
	while( *path != 0 ){
		if ( *path == '/' ){
			tmp++;
			name = path+1;
		}
		path++;
	}

	if( elements != NULL ){
		*elements = tmp;
	}
	return name;
}


/*! \details This finds the filesystem associated with a path.
 *
 */
const sysfs_t * sysfs_find(const char * path, bool needs_parent){
	return NULL;
}

const char * sysfs_stripmountpath(const sysfs_t * fs, const char * path){
	path = path + strlen(fs->mount_path);
	if (path[0] == '/' ){
		path++;
	}
	return path;
}

static bool isinvalid(const char * path, int max){
	int len;
	len = strnlen(path, max);
	if ( len == max ){
		errno = ENAMETOOLONG;
		return true;
	}

	if ( len != strspn(path, sysfs_validset) ){
		errno = EINVAL;
		return true;
	}

	return false;
}

bool sysfs_ispathinvalid(const char * path){
	return isinvalid(path, PATH_MAX);
}

bool sysfs_isvalidset(const char * path){
	if ( strlen(path) == strspn(path, sysfs_validset)){
		return true;
	}
	return false;
}

int sysfs_getamode(int flags){
	int accmode;
	int amode;
	accmode = flags & O_ACCMODE;
	amode = 0;
	switch(accmode){
	case O_RDWR:
		amode = R_OK | W_OK;
		break;
	case O_WRONLY:
		amode = W_OK;
		break;
	case O_RDONLY:
		amode = R_OK;
		break;
	}
	if ( (flags & O_CREAT) || (flags & O_TRUNC) ){
		amode |= W_OK;
	}
	return amode;
}

int sysfs_access(int file_mode, int file_uid, int file_gid, int amode){
	int is_ok;
	uid_t euid;
	gid_t egid;

	euid = geteuid();
	egid = getegid();

	is_ok = 0;
	if ( amode & R_OK ){
		if ( file_mode & S_IROTH ){
			is_ok |= R_OK;
		} else if ( (file_mode & S_IRUSR)  && ( file_uid == euid) ){
			//Check to see if s.st_uid matches current user id
			is_ok |= R_OK;
		} else if ( (file_mode & S_IRGRP) && ( file_gid == egid) ){
			//Check to see if gid matches current group id
			is_ok |= R_OK;
		}
	}

	if ( amode & W_OK ){
		if ( file_mode & S_IWOTH ){
			is_ok |= W_OK;
		} else if ( (file_mode & S_IWUSR) && ( file_uid == euid) ){
			//Check to see if user id matches file_uid
			is_ok |= W_OK;
		} else if ( (file_mode & S_IWGRP) && ( file_gid == egid) ){
			//Check to see if gid matches current group id
			is_ok |= W_OK;
		}
	}

	if ( amode & X_OK){
		if ( file_mode & S_IXOTH ){
			is_ok |= X_OK;
		} else if ( (file_mode & S_IXUSR) && ( file_uid == euid)  ){
			//Check to see if s.st_uid matches current user id
			is_ok |= X_OK;
		} else if ( (file_mode & S_IXGRP) && ( file_gid == egid) ){
			//Check to see if gid matches current group id
			is_ok |= X_OK;
		}
	}

	if ( is_ok == amode ){
		return 0;
	}
	return -1;
}




int sysfs_notsup(){
	errno = ENOTSUP;
	return -1;
}

void * sysfs_notsup_null(){
	errno = ENOTSUP;
	return NULL;
}

void sysfs_notsup_void(){}
