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

/*! \addtogroup SYSFS
 * @{
 */

/*! \file */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "mcu/debug.h"
#include "sos/fs/sysfs.h"

#define OR_ALLOW_GROUP 0

const char sysfs_validset[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_./";
const char sysfs_whitespace[] =
		" \t\r\n";

int mkfs(const char * path){
	const sysfs_t * fs;

	fs = sysfs_find(path, false);
	if( fs == NULL ){
		errno = ENOENT;
		return -1;
	}

	int ret = fs->mkfs(fs->config);
	SYSFS_PROCESS_RETURN(ret);
	return ret;
}

int mount(const char * path){
	const sysfs_t * fs;

	fs = sysfs_find(path, false);
	if( fs == NULL ){
		errno = ENOENT;
		return -1;
	}

	int ret =  fs->mount(fs->config);
	SYSFS_PROCESS_RETURN(ret);
	return ret;
}

int unmount(const char * path){
	const sysfs_t * fs;
	fs = sysfs_find(path, false);
	if( fs == NULL ){
		errno = ENOENT;
		return -1;
	}
	int ret =  fs->unmount(fs->config);
	SYSFS_PROCESS_RETURN(ret);
	return ret;
}

int sysfs_always_mounted(const void * cfg){
	return 1;
}

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

const char * sysfs_get_filename(const char * path){
	const char * name;
	name = path;
	while( *path != 0 ){
		if ( *path == '/' ){
			name = path+1;
		}
		path++;
	}
	return name;
}


/*! \details This finds the filesystem associated with a path.
 *
 */
const sysfs_t * sysfs_find(const char * path, bool needs_parent){
	int i;
	int pathlen;
	int mountlen;
	pathlen = strlen(path);

	i = 0;
	while( sysfs_isterminator(&(sysfs_list[i])) == false ){
		mountlen = strlen(sysfs_list[i].mount_path);
		if( strncmp(path, sysfs_list[i].mount_path, mountlen) == 0 ){
			if ( needs_parent == true ){
				if ( (pathlen > (mountlen+1)) || (pathlen == 1) ){
					return &sysfs_list[i];
				}
			} else {
				return &sysfs_list[i];
			}
		}
		i++;
	}
	return NULL;
}

const char * sysfs_stripmountpath(const sysfs_t * fs, const char * path){
	path = path + strlen(fs->mount_path);
	if (path[0] == '/' ){
		path++;
	}
	return path;
}

static bool isinvalid(
		const char * path,
		int max
		){
	int len;
	int tmp;
	const char * p;
	len = strnlen(path, max);
	if ( len == max ){
		errno = ENAMETOOLONG;
		return true;
	}

	p = path;
	tmp = 0;
	do {
		p++;
		if ( (*p == '/') || (*p == 0) ){
			if ( tmp > NAME_MAX ){
				errno = ENAMETOOLONG;
				return true;
			}
			tmp = 0;
		}
		tmp++;
	} while( *p != 0 );


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
	int len = strnlen(path, PATH_MAX);
	if( len == PATH_MAX ){
		return false;
	}

	if ( len == strspn(path, sysfs_validset)){
		return true;
	}
	return false;
}

int sysfs_getamode(int flags){
	int accmode;
	int amode;
	amode = 0;
	accmode = flags & O_ACCMODE;
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

int sysfs_is_r_ok(int file_mode, int file_uid, int file_gid){
	if ( file_mode & S_IROTH ){
		return 1;
	} else if ( (file_mode & S_IRUSR)  && (file_uid == getuid() || getuid() == SYSFS_ROOT) ){
		//Check to see if s.st_uid matches current user id
		return 1;
	}
#if OR_ALLOW_GROUP
	else if ( (file_mode & S_IRGRP) && ( file_gid == getgid()) ){
		//Check to see if gid matches current group id
		return 1;
	}
#else
	MCU_UNUSED_ARGUMENT(file_gid);
#endif
	return 0;
}

int sysfs_is_w_ok(int file_mode, int file_uid, int file_gid){
	if ( file_mode & S_IWOTH ){
		return 1;
	} else if ( (file_mode & S_IWUSR) && (file_uid == getuid() || getuid() == SYSFS_ROOT) ){
		//Check to see if user id matches file_uid
		return 1;
	}
#if OR_ALLOW_GROUP
	else if ( (file_mode & S_IWGRP) && ( file_gid == getgid()) ){
		//Check to see if gid matches current group id
		return 1;
	}
#else
	MCU_UNUSED_ARGUMENT(file_gid);
#endif
	return 0;
}

int sysfs_is_rw_ok(int file_mode, int file_uid, int file_gid){
	int is_ok = 0;
	if ( file_mode & S_IWOTH ){
		is_ok = W_OK;
	} else if ( (file_mode & S_IWUSR) &&
					(file_uid == getuid() ||
					 getuid() == SYSFS_ROOT) ){
		//Check to see if user id matches file_uid
		is_ok = W_OK;
	}
#if OR_ALLOW_GROUP
	else if ( (file_mode & S_IWGRP) && ( file_gid == getgid()) ){
		//Check to see if gid matches current group id
		is_ok = W_OK;
	}
#else
	MCU_UNUSED_ARGUMENT(file_gid);
#endif

	if( is_ok == W_OK ){
		if ( file_mode & S_IROTH ){
			return 1;
		} else if ( (file_mode & S_IRUSR) &&
						(file_uid == getuid() ||
						 getuid() == SYSFS_ROOT) ){
			//Check to see if s.st_uid matches current user id
			return 1;
		}
#if OR_ALLOW_GROUP
		else if ( (file_mode & S_IRGRP) && ( file_gid == getgid()) ){
			//Check to see if gid matches current group id
			return 1;
		}
#endif
	}
	return 0;
}


int sysfs_is_x_ok(int file_mode, int file_uid, int file_gid){
	if ( file_mode & S_IXOTH ){
		return 1;
	} else if ( (file_mode & S_IXUSR) && (file_uid == getuid() || getuid() == SYSFS_ROOT)  ){
		//Check to see if s.st_uid matches current user id
		return 1;
	}
#if OR_ALLOW_GROUP
	else if ( (file_mode & S_IXGRP) && ( file_gid == getgid()) ){
		//Check to see if gid matches current group id
		return 1;
	}
#endif
	return 0;
}

int sysfs_access(int file_mode, int file_uid, int file_gid, int amode){
	int is_ok = 0;

	if ( amode & R_OK ){
		if( sysfs_is_r_ok(file_mode, file_uid, file_gid) ){
			is_ok |= R_OK;
		}
	}

	if ( amode & W_OK ){
		if( sysfs_is_w_ok(file_mode, file_uid, file_gid) ){
			is_ok |= W_OK;
		}
	}

	if ( amode & X_OK){
		if( sysfs_is_x_ok(file_mode, file_uid, file_gid) ){
			is_ok |= X_OK;
		}
	}

	if ( is_ok == amode ){
		return 0;
	}

	return -1;
}

void sysfs_unlock(){
	int i;
	i = 0;
	while( sysfs_isterminator(&(sysfs_list[i])) == false ){
		sysfs_list[i].unlock( sysfs_list[i].config );
		i++;
	}
}



int sysfs_notsup(){
	return SYSFS_SET_RETURN(ENOTSUP);
}

void * sysfs_notsup_null(){
	errno = ENOTSUP;
	return NULL;
}

void sysfs_notsup_void(){}
