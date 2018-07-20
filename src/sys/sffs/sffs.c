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

#include <sys/stat.h>

#ifndef __SIM__
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "sffs_local.h"
#include "sffs_serialno.h"
#include "sffs_dir.h"
#include "sffs_file.h"
#include "sffs_block.h"
#include "sffs_scratch.h"
#include "sos/fs/sffs.h"
#include "sos/fs/sysfs.h"

extern int pthread_mutex_force_unlock(pthread_mutex_t *mutex);

#define OPENDIR_HANDLE ((void*)0x1234567)


#define DEBUG_LEVEL 1

void sffs_unlock(const void * config){ //force unlock when a process exits
#ifndef __SIM__
    pthread_mutex_force_unlock(SFFS_DRIVE_MUTEX(config));
#endif
}

static void lock_sffs(const sffs_config_t * config){
#ifndef __SIM__
    if ( pthread_mutex_lock(SFFS_DRIVE_MUTEX(config)) < 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "Failed to lock sffs %d", errno);
	}
    sffs_dev_setdelay_mutex(SFFS_DRIVE_MUTEX(config));
#endif
}

static void unlock_sffs(const sffs_config_t * config){
	sffs_dev_setdelay_mutex(NULL);
#ifndef __SIM__
    if ( pthread_mutex_unlock(SFFS_DRIVE_MUTEX(config)) < 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "Failed to unlock sffs %d", errno);
	}
#endif
}


int sffs_unmount(const void * cfg){
	//close the device access file descriptor
	return sffs_dev_close(cfg);
}

int sffs_ismounted(const void * cfg){
	//check to see if device file descriptor is open
    if( SFFS_CONFIG(cfg)->drive.state->file.handle == 0 ){
		return 0;
	}
	return 1;
}

int sffs_init(const void * cfg){
	cl_snlist_item_t bad_serialno;
	int err;
	int tmp;
	int bad_files;
	bool clean_open_blocks;
#ifndef __SIM__
	pthread_mutexattr_t mutexattr;

	if( pthread_mutexattr_init(&mutexattr) < 0 ){
		return -1;
	}

	pthread_mutexattr_setpshared(&mutexattr, true);
	pthread_mutexattr_setprioceiling(&mutexattr, 19);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

    if ( pthread_mutex_init(SFFS_DRIVE_MUTEX(cfg), &mutexattr) ){
		return -1;
	}
#endif

	if ( sffs_dev_open(cfg) < 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "Failed to open dev");
		return -1;
	}

	bad_files = 0;
	clean_open_blocks = false;

	while( (err = sffs_serialno_init(cfg, &bad_serialno)) == 1 ){

		if( (tmp = sffs_file_clean(cfg, bad_serialno.serialno, bad_serialno.block, bad_serialno.status)) < 0 ){
            mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to clean file");
			return -1;
		}

		if ( tmp == 1 ){
			clean_open_blocks = true;
		}

		bad_files++;
		if ( bad_files > 4 ){
			err = -1;
			break;
		}
	}

	if ( err == -1 ){
		//failed to find initial serial numbers so no other access is allowed
		if( sffs_mkfs(cfg) < 0 ){
            mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "Failed to format");
		} else {
            mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "Format complete");
		}

        SFFS_CONFIG(cfg)->drive.state->file.fs = 0;
		return -1;
	}

	if ( sffs_scratch_init(cfg) < 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to restore scratch area");
		return -1;
	}

	if ( clean_open_blocks == true ){
		//scan all blocks and discard "OPEN" blocks
		if ( sffs_block_discardopen(cfg) < 0 ){
            mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to discard open blocks");
			return -1;
		}
	}

    sffs_debug(DEBUG_LEVEL, "Found %d bad files", bad_files);

	//start a new thread to handle reads/writes if asynchronous IO will be supported
	return 0;
}


int sffs_mkfs(const void * cfg){
	int ret;
	const sffs_config_t * cfgp = cfg;
	lock_sffs(cfgp);
	ret = 0;
    SFFS_CONFIG(cfg)->drive.state->file.fs = SFFS_CONFIG(cfg)->drive.devfs;
    sffs_debug(DEBUG_LEVEL, "Erase device");
	if ( (ret = sffs_dev_erase(cfg)) < 0 ){
        SFFS_CONFIG(cfg)->drive.state->file.fs = NULL;
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to erase");
	} else {
        sffs_debug(DEBUG_LEVEL, "Init serial number");
		if ( (ret = sffs_serialno_mkfs(cfg)) < 0 ){
			//failed to format so no other access is allowed
            SFFS_CONFIG(cfg)->drive.state->file.fs = NULL;
		}
	}
	unlock_sffs(cfgp);
	return ret;
}

int sffs_fstat(const void * cfg, void * handle, struct stat * stat){
	cl_handle_t * h = (cl_handle_t*)handle;
	sffs_block_data_t tmp;
	int ret;
	ret = 0;

	lock_sffs(cfg);
	if ( (ret = sffs_block_load(cfg, h->hdr_block, &tmp)) < 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to load header block");
        ret = SYSFS_SET_RETURN(EIO);
	} else {
        stat->st_ino = (ino_t)h->segment_data.hdr.serialno;
		stat->st_size = h->size;
        stat->st_blocks = ((h->size + BLOCK_DATA_SIZE - 1) / BLOCK_DATA_SIZE);
		stat->st_blksize = BLOCK_DATA_SIZE;
		stat->st_mode = 0666 | S_IFREG;
		//stat->st_atime = 0;
		stat->st_ctime = 0;
		stat->st_mtime = 0;
	}

	unlock_sffs(cfg);
	return ret;
}

int sffs_stat(const void * cfg, const char * path, struct stat * stat){
	cl_handle_t handle;
	sffs_dir_lookup_t entry;
	int ret;

	CL_TP(CL_PROB_RARE);

	lock_sffs(cfg);
    sffs_debug(DEBUG_LEVEL, "path:%s", path);
	ret = sffs_dir_exists(cfg, path, &entry, R_OK);
	if ( ret < 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to check existence");
	} else if ( ret != SFFS_DIR_PATH_EXISTS ){
		//path does not exist
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "path does not exist");
        ret = SYSFS_SET_RETURN(ENOENT);
	} else {
		//open the file
        sffs_debug(DEBUG_LEVEL, "open file");
		if ( sffs_file_open(cfg, &handle, entry.serialno, R_OK, false) < 0 ){
            mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to open file");
			ret = -1;
		} else {
			//call fstat
            sffs_debug(DEBUG_LEVEL, "run stat");
			if( sffs_fstat(cfg, &handle, stat) < 0 ){
				//cl_fstat() will set the error number
                mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to run fstat");
				ret = -1;
			}
			ret = 0;
		}
	}
	unlock_sffs(cfg);
	return ret;
}

int sffs_unlink(const void * cfg, const char * path){
	sffs_dir_lookup_t entry;
	int ret;

	lock_sffs(cfg);

	ret = sffs_dir_exists(cfg, path, &entry, W_OK);
	if ( ret < 0 ){
		ret = -1;
		goto sffs_unlink_unlock;
	}

	if ( ret != SFFS_DIR_PATH_EXISTS ){
		//path does not exist
        ret = SYSFS_SET_RETURN(ENOENT);
		goto sffs_unlink_unlock;
	}

	ret = 0;

    sffs_debug(DEBUG_LEVEL, "unlink serialno %d", entry.serialno);

	if ( sffs_serialno_get(cfg, entry.serialno, SFFS_SNLIST_ITEM_STATUS_OPEN, NULL) != BLOCK_INVALID ){
		//the file is open -- cannot delete
        ret = SYSFS_SET_RETURN(EACCES);
		goto sffs_unlink_unlock;
	}

	//clean up the file system
	if ( sffs_file_remove(cfg, entry.serialno) < 0 ){
		ret = -1;
		goto sffs_unlink_unlock;
	}

	sffs_unlink_unlock:
	unlock_sffs(cfg);
	return ret;
}

int sffs_remove(const void * cfg, const char * path){
	CL_TP(CL_PROB_IMPROBABLE);
	return sffs_unlink(cfg, path);
}

int sffs_open(const void * cfg, void ** handle, const char * path, int flags, int mode){
	int err;
	int ret;
	cl_handle_t * h;
	sffs_dir_lookup_t entry;
	const char * name;
	int amode;

	amode = sysfs_getamode(flags);

	lock_sffs(cfg);

	//lock
	if ( (err = sffs_dir_exists(cfg, path, &entry, amode)) < 0 ){
		//unlock
        ret = SYSFS_SET_RETURN(ENOTDIR);
		h = NULL;
		goto sffs_open_unlock;
	}

    sffs_debug(DEBUG_LEVEL, "Open serialno %d (%d)", entry.serialno, err);

	//See if there is a slot for an open file
	h = malloc(sizeof(cl_handle_t));
	if ( h == NULL ){
        ret = SYSFS_SET_RETURN(ENOMEM);
		goto sffs_open_unlock;
	}

	ret = 0;
	name = sysfs_getfilename(path, NULL);
	if ( err == SFFS_DIR_PATH_EXISTS ){
		//The file already exists
		if ( (flags & O_EXCL) && (flags & O_CREAT) ){
			//it is an error to try to exclusively create a file that already exists
            ret = SYSFS_SET_RETURN(EEXIST);
		} else if ( flags & O_TRUNC ){

			//truncate file (use the same serialno)
            sffs_debug(DEBUG_LEVEL, "create new file (old file truncated on close)");
			if ( sffs_file_open(cfg, h, entry.serialno, amode, true) < 0 ){
                ret = SYSFS_SET_RETURN(ENOSPC);
			}

		} else {
			//open the existing file
			if ( sffs_file_open(cfg, h, entry.serialno, amode, false) < 0 ){
                ret = SYSFS_SET_RETURN(ENOSPC);
			} else  if ( h->size < 0 ){
				//There is an error with this file -- it is likely corrupt
                ret = SYSFS_SET_RETURN(EFBIG);
			}
		}
	} else if ( err == SFFS_DIR_PARENT_EXISTS ){ //does the path exist?
		if ( flags & (O_CREAT) ){
			//create a new file
            //printf("Create brand new file");
			entry.serialno = sffs_serialno_new(cfg);
			if ( sffs_file_new(cfg, h, name, mode, &entry, BLOCK_TYPE_FILE_HDR, amode) < 0 ){
                ret = SYSFS_SET_RETURN(ENOSPC);
			}

		} else {
            sffs_debug(DEBUG_LEVEL, "file %s does not exist", path);
            ret = SYSFS_SET_RETURN(ENOENT);
		}
	} else {
		//Path does not exist
        sffs_debug(DEBUG_LEVEL, "file %s does not exist here", path);
        ret = SYSFS_SET_RETURN(ENOENT);
	}

	//unlock()
	if ( ret == -1 ){
		free(h);
		h = NULL;
	}

	CL_TP_DESC(CL_PROB_IMPROBABLE, "file has been opened");
	sffs_open_unlock:
	*handle = h;
	unlock_sffs(cfg);
	return ret;
}

int sffs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte){
    MCU_UNUSED_ARGUMENT(flags);
	cl_handle_t * h = (cl_handle_t*)handle;
	devfs_async_t op;
	int ret;

	h->op = &op;

	op.loc = loc;
	op.buf = buf;
	op.nbyte = nbyte;
	op.handler.callback = NULL;
	op.handler.context = NULL;

	CL_TP(CL_PROB_IMPROBABLE);

    sffs_debug(DEBUG_LEVEL + 5, "read %d bytes", nbyte);

	if ( (h->amode & R_OK) == 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "no read access");
        return SYSFS_SET_RETURN(EACCES);
	}

	if ( sffs_file_startread(cfg, handle) == nbyte ){
		return op.nbyte;
	}

	lock_sffs(cfg);

	if ( sffs_file_finishread(cfg, handle) < 0 ){
		ret = -1;
	} else {
		ret = op.nbyte;
	}

	unlock_sffs(cfg);
	return ret;
}


int sffs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte){
	cl_handle_t * h = (cl_handle_t*)handle;
	devfs_async_t op;
	int ret;

	h->op = &op;

	if ( flags & O_APPEND ){
		loc = h->size;
	}

	op.loc = loc;
	op.buf_const = buf;
	op.nbyte = nbyte;
	op.handler.callback = NULL;
	op.handler.context = NULL;

	CL_TP(CL_PROB_IMPROBABLE);

    sffs_debug(DEBUG_LEVEL + 5, "write %d bytes", nbyte);

	if ( (h->amode & W_OK) == 0 ){
        mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "no write access");
        return SYSFS_SET_RETURN(EACCES);
	}

	if ( sffs_file_startwrite(cfg, handle) == nbyte ){
		return op.nbyte;
	}

	lock_sffs(cfg);
	if ( sffs_file_finishwrite(cfg, handle) < 0 ){
		ret = -1;
	} else {
		ret = op.nbyte;
	}
	unlock_sffs(cfg);
	return ret;
}

int sffs_close(const void * cfg, void ** handle){
	int ret;
	void * h;
	CL_TP(CL_PROB_COMMON);
	h = *handle;
	lock_sffs(cfg);
	ret = sffs_file_close(cfg, h);
	*handle = NULL;
	free(h);
	unlock_sffs(cfg);
	if( ret < 0 ){
        ret = SYSFS_SET_RETURN(EIO);
	}
	return ret;
}

int sffs_opendir(const void * cfg, void ** handle, const char * path){
    MCU_UNUSED_ARGUMENT(cfg);
	if( path[0] != 0 ){
        return SYSFS_SET_RETURN(ENOTDIR);
	}

	*handle = OPENDIR_HANDLE;
	return 0;
}

int sffs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry){
	sffs_block_data_t hdr_sffs_block_data;
	cl_hdr_t * hdr;
	sffs_list_t sn_list;
	cl_snlist_item_t item;
	int count;
	int ret;

	if ( handle != OPENDIR_HANDLE ){
        return SYSFS_SET_RETURN(EBADF);
    }

	lock_sffs(cfg);

    sffs_debug(DEBUG_LEVEL, "initialize list");
	if ( cl_snlist_init(cfg, &sn_list, sffs_serialno_getlistblock(cfg) ) < 0 ){
        ret = SYSFS_SET_RETURN(EIO);
		goto sffs_readdir_unlock;
	}

	count = 0;
	hdr = (cl_hdr_t *)hdr_sffs_block_data.data;
	ret = -1;
	while( cl_snlist_getnext(cfg, &sn_list, &item) == 0 ){
		if( (item.status == SFFS_SNLIST_ITEM_STATUS_CLOSED) && (item.serialno != CL_SERIALNO_LIST) ){
			if ( count == loc ){
				if ( sffs_block_load(cfg, item.block, &hdr_sffs_block_data) ){
                    mcu_debug_log_error(MCU_DEBUG_FILESYSTEM, "failed to load block %d for serialno:%d", item.block, item.serialno);
                    ret = SYSFS_SET_RETURN(EIO);
					goto sffs_readdir_unlock;
				}

				strcpy(entry->d_name, hdr->open.name);
                entry->d_ino = (ino_t)item.serialno;
				ret = 0;
				goto sffs_readdir_unlock;
			}
			count++;
		}
	}

	sffs_readdir_unlock:
	unlock_sffs(cfg);
	return ret;
}


int sffs_closedir(const void * cfg, void ** handle){
    MCU_UNUSED_ARGUMENT(cfg);
	if ( *handle != OPENDIR_HANDLE ){
        return SYSFS_SET_RETURN(EBADF);
	}
	*handle = 0;
	return 0;
}
