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

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "dirent.h"
#include "mcu/types.h"
#include "sos/sos.h"
#include "sos/fs/sysfs.h"
#include "sos/fs/assetfs.h"
#include "cortexm/cortexm.h"

#define INVALID_DIR_HANDLE ((void*)0)
#define VALID_DIR_HANDLE ((void*)0x12345678)

typedef struct {
	int ino;
	const void * data;
	u32 size;
	u32 checksum;
} assetfs_handle_t;

int assetfs_init(const void* cfg){
	//nothing to initialize
	return 0;
}


static int get_directory_entry(const void * cfg, int loc, const assetfs_dirent_t ** entry);
static const assetfs_dirent_t * find_file(const void * cfg, const char * path, int * ino);
static void assign_stat(int ino, const assetfs_dirent_t * entry, struct stat * st);

int assetfs_startup(const void* cfg){
	//check for any applications that are embedded and start them?
	return 0;
}

int assetfs_open(const void* cfg, void ** handle, const char * path, int flags, int mode){
	MCU_UNUSED_ARGUMENT(mode);

	if( flags != O_RDONLY ){ return SYSFS_SET_RETURN(EINVAL); }

	int ino;
	const assetfs_dirent_t * directory_entry =
			find_file(cfg, path, &ino);

	if( sysfs_is_r_ok(directory_entry->mode, directory_entry->uid, SYSFS_GROUP) == 0 ){
		return SYSFS_SET_RETURN(EPERM);
	}

	assetfs_handle_t * h = malloc(sizeof(assetfs_handle_t));
	if( h == 0 ){
		return -1;
	}

	h->ino = ino;
	h->data = directory_entry->start;
	h->size = directory_entry->end - directory_entry->start;
	cortexm_assign_zero_sum32(h, sizeof(assetfs_handle_t) / sizeof(u32));

	*handle = h;
	return 0;
}

int assetfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte){
	MCU_UNUSED_ARGUMENT(cfg);
	if( flags != O_RDONLY ){ return SYSFS_SET_RETURN(EINVAL); }
	assetfs_handle_t * h = handle;
	if( cortexm_verify_zero_sum32(h, sizeof(assetfs_handle_t) / sizeof(u32)) == 0 ){
		return SYSFS_SET_RETURN(EINVAL);
	}
	if( loc < 0 ){ return SYSFS_SET_RETURN(EINVAL); }
	int bytes_ready = h->size - loc;
	if( bytes_ready > nbyte ){ bytes_ready = nbyte; }
	if( bytes_ready <= 0 ){ return 0; }
	//don't read past the end of the file
	memcpy(buf, h->data + loc, bytes_ready);
	return bytes_ready;

}

int assetfs_ioctl(
		const void * cfg,
		void * handle,
		int request,
		void * ctl
		){
	MCU_UNUSED_ARGUMENT(cfg);
	MCU_UNUSED_ARGUMENT(handle);
	MCU_UNUSED_ARGUMENT(request);
	MCU_UNUSED_ARGUMENT(ctl);
	errno = ENOTSUP;
	return -1;
}

int assetfs_close(const void* cfg, void ** handle){
	MCU_UNUSED_ARGUMENT(cfg);
	if( *handle != 0 ){
		if( cortexm_verify_zero_sum32(*handle, sizeof(assetfs_handle_t) / sizeof(u32)) == 0 ){
			return SYSFS_SET_RETURN(EINVAL);
		}
		free(*handle);
		*handle = 0;
	}
	return 0;
}

int assetfs_fstat(const void* cfg, void * handle, struct stat * st){
	MCU_UNUSED_ARGUMENT(cfg);
	assetfs_handle_t * h = handle;
	const assetfs_dirent_t * directory_entry;
	const assetfs_config_t * config = cfg;
	if( cortexm_verify_zero_sum32(h, sizeof(assetfs_handle_t) / sizeof(u32)) == 0 ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	directory_entry = config->entries + h->ino;
	assign_stat(h->ino, directory_entry, st);

	return 0;

}

int assetfs_stat(const void* cfg, const char * path, struct stat * st){
	int ino;
	const assetfs_dirent_t * directory_entry = find_file(cfg, path, &ino);
	if( directory_entry == 0 ){
		return SYSFS_SET_RETURN(ENOENT);
	}

	assign_stat(ino, directory_entry, st);
	return 0;

}

void assign_stat(int ino, const assetfs_dirent_t * entry, struct stat * st){
	memset(st, 0, sizeof(struct stat));
	st->st_size = entry->end - entry->start;
	st->st_ino = ino;
	st->st_mode = entry->mode | S_IFREG;
	st->st_uid = SOS_USER_ROOT;
}

int assetfs_opendir(const void* cfg, void ** handle, const char * path){
	if ( strncmp(path, "", PATH_MAX) == 0 ){
		*handle = VALID_DIR_HANDLE;
	} else {
		return SYSFS_SET_RETURN(ENOENT);
	}
	return 0;
}

int assetfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry){
	if( handle != VALID_DIR_HANDLE ){ return SYSFS_SET_RETURN(EINVAL); }

	const assetfs_dirent_t * directory_entry = 0;
	int result = get_directory_entry(cfg, loc, &directory_entry);
	if( result < 0 ){ return result; }

	entry->d_name[NAME_MAX-1] = 0;
	strncpy(entry->d_name, directory_entry->name, NAME_MAX-1);
	entry->d_ino = loc;

	return 0;
}

int assetfs_closedir(const void* cfg, void ** handle){
	if( handle != VALID_DIR_HANDLE ){ return SYSFS_SET_RETURN(EINVAL); }
	*handle = INVALID_DIR_HANDLE;
	return 0;
}

const assetfs_dirent_t * find_file(const void * cfg, const char * path, int * ino){
	int loc = 0;
	const assetfs_dirent_t * directory_entry = 0;

	while( get_directory_entry(cfg, loc, &directory_entry) == 0){
		if( strncmp(path, directory_entry->name, NAME_MAX-1) == 0 ){
			*ino = loc;
			return directory_entry;
		}
		loc++;
	}

	return 0;
}

int get_directory_entry(const void * cfg, int loc, const assetfs_dirent_t ** entry){
	const assetfs_config_t * config = cfg;
	if( loc < 0 ){ return SYSFS_SET_RETURN(EINVAL);	}
	if( loc >= config->count ){ return SYSFS_SET_RETURN(ENOENT); }
	*entry = config->entries + loc;
	return 0;

}
