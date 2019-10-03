/* Copyright 2011-2017 Tyler Gilbert;
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

#include <fcntl.h>
#include <unistd.h>
#include "mcu/debug.h"
#include "sos/fs/sysfs.h"

extern int devfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode);
static void update_loc(sysfs_file_t * file, int adjust);

int sysfs_file_open(sysfs_file_t * file, const char * name, int mode){
    int ret;
    struct stat st;
    const sysfs_t * fs = file->fs;

	 int access_mode = sysfs_getamode(mode);
	 //check fs level permissions
	 if( sysfs_access(
			  fs->permissions,
			  fs->owner,
			  SYSFS_GROUP,
			  access_mode
			  ) < 0 ){
		 errno = EPERM;
		 return -1*__LINE__;
	 }

    ret = fs->open(fs->config, &(file->handle), name, file->flags, mode);
    if( ret >= 0 ){
        if( fs->open == devfs_open ){
            if( (ret = fs->fstat(fs->config, file->handle, &st)) == 0 ){
                if( (st.st_mode & S_IFMT) == S_IFCHR ){
                    file->flags |= O_CHAR;
                }
            }
        }
    }
    SYSFS_PROCESS_RETURN(ret);
    return ret;
}

int sysfs_file_read(sysfs_file_t * file, void * buf, int nbyte){
    const sysfs_t * fs = file->fs;
    int bytes;
    bytes = fs->read(fs->config, file->handle, file->flags, file->loc, buf, nbyte);
    SYSFS_PROCESS_RETURN(bytes);
    update_loc(file, bytes);
    return bytes;
}

int sysfs_file_write(sysfs_file_t * file, const void * buf, int nbyte){
    const sysfs_t * fs = file->fs;
    int bytes;
    bytes = fs->write(fs->config, file->handle, file->flags, file->loc, buf, nbyte);
    SYSFS_PROCESS_RETURN(bytes);
    update_loc(file, bytes);
    return bytes;
}

int sysfs_file_aio(sysfs_file_t * file, void * aiocbp){
    const sysfs_t * fs = file->fs;
    int ret =  fs->aio(fs->config, file->handle, aiocbp);
    SYSFS_PROCESS_RETURN(ret);
    return ret;
}


int sysfs_file_close(sysfs_file_t * file){
    const sysfs_t * fs = file->fs;
    int ret = fs->close(fs->config, &file->handle);
    SYSFS_PROCESS_RETURN(ret);
    return ret;
}

int sysfs_file_ioctl(sysfs_file_t * file, int request, void * ctl){
    const sysfs_t * fs = file->fs;
    int ret = fs->ioctl(fs->config, file->handle, request, ctl);
    SYSFS_PROCESS_RETURN(ret);
    return ret;
}

int sysfs_file_fsync(sysfs_file_t * file){
    const sysfs_t * fs = file->fs;
    int ret = fs->fsync(fs->config, file->handle);
    SYSFS_PROCESS_RETURN(ret);
    return ret;
}

void update_loc(sysfs_file_t * file, int adjust){
    //if not a char device, increment the location
    if ( (adjust > 0) && ((file->flags & O_CHAR) == 0) ){
        file->loc += adjust;
    }
}

int sysfs_shared_open(const sysfs_shared_config_t * config){
    config->state->file.fs = config->devfs;
    config->state->file.flags = O_RDWR;
    config->state->file.loc = 0;
    config->state->file.handle = NULL;
	 return sysfs_file_open(&(config->state->file), config->name, O_RDWR);
}

int sysfs_shared_ioctl(const sysfs_shared_config_t * config, int request, void * ctl){
    if( config->state->file.fs == 0 ){ return SYSFS_SET_RETURN(ENODEV); }
	 return sysfs_file_ioctl(&(config->state->file), request, ctl);
}

int sysfs_shared_fsync(const sysfs_shared_config_t * config){
    return 0;
}

int sysfs_shared_read(const sysfs_shared_config_t * config, int loc, void * buf, int nbyte){
    if( config->state->file.fs == 0 ){ return SYSFS_SET_RETURN(ENODEV); }
    config->state->file.loc = loc;
	 return sysfs_file_read(&(config->state->file), buf, nbyte);
}

int sysfs_shared_write(const sysfs_shared_config_t * config, int loc, const void * buf, int nbyte){
    if( config->state->file.fs == 0 ){ return SYSFS_SET_RETURN(ENODEV); }
    config->state->file.loc = loc;
	 return sysfs_file_write(&(config->state->file), buf, nbyte);
}

int sysfs_shared_aio(const sysfs_shared_config_t * config, void * aio){
    if( config->state->file.fs == 0 ){ return SYSFS_SET_RETURN(ENODEV); }
	 return sysfs_file_aio(&(config->state->file), aio);
}

int sysfs_shared_close(const sysfs_shared_config_t * config){
    if( config->state->file.fs == 0 ){ return SYSFS_SET_RETURN(ENODEV); }
	 return sysfs_file_close(&(config->state->file));
}


