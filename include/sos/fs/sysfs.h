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


#ifndef SYSFS_H_
#define SYSFS_H_

#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/lock.h>

#if !defined __link
#include "types.h"
#include "aio.h"
#endif

#if defined __cplusplus
extern "C" {
#endif


//Encodes the error number and the line number ex: return SYSFS_SET_RETURN(EINVAL);
#define SYSFS_SET_RETURN(error_number) (-1*(error_number | (__LINE__ << 8)))
#define SYSFS_SET_RETURN_WITH_VALUE(error_number, value) (-1*(error_number | (value << 8)))
#define SYSFS_RETURN_SUCCESS (0)
#define SYSFS_RETURN_EOF (-1)

#define SYSFS_GET_RETURN_ERRNO(value) ( (-1*value) & 0xff)
#define SYSFS_GET_RETURN(value) ( -1*( (-1* value) >> 8))

//Takes a value created using SYSFS_SET_RETURN() and pulls out the errno (assigns to errno) then assigns the __LINE__ number to value
#define SYSFS_PROCESS_RETURN(value) do { if( value < -1 ) { errno = SYSFS_GET_RETURN_ERRNO(value); value = SYSFS_GET_RETURN(value); } } while(0)

#if !defined __link


int sysfs_notsup();
void * sysfs_notsup_null();
void sysfs_notsup_void();
int sysfs_always_mounted(const void *);

#define SYSFS_NOTSUP ((void*)sysfs_notsup)
#define SYSFS_NOTSUP_VOID ((void*)sysfs_notsup_void)

typedef struct {
    const char mount_path[PATH_MAX];
    const int access;
    int (*mount)(const void*);
    int (*unmount)(const void*);
    int (*ismounted)(const void*);
    int (*startup)(const void*);
    int (*mkfs)(const void*);
    int (*open)(const void*, void**, const char*, int, int);
    int (*aio)(const void*, void*, struct aiocb*);
    int (*ioctl)(const void*, void*, int, void*);
    int (*read)(const void*, void*, int, int, void*, int);
    int (*write)(const void*, void*, int, int, const void*, int);
    int (*fsync)(const void*, void*);
    int (*close)(const void*, void**);
    int (*fstat)(const void*, void*, struct stat*);
    int (*rename)(const void*, const char*, const char*);
    int (*unlink)(const void*, const char*);
    int (*mkdir)(const void*, const char*, mode_t);
    int (*rmdir)(const void*, const char*);
    int (*remove)(const void*, const char*);
    int (*opendir)(const void*, void**, const char*);
    int (*closedir)(const void*, void**);
    int (*readdir_r)(const void*, void*, int, struct dirent*);
    int (*link)(const void*, const char *, const char*);
    int (*symlink)(const void*, const char *, const char*);
    int (*stat)(const void*, const char *, struct stat*);
    int (*lstat)(const void*, const char *, struct stat*);
    int (*chmod)(const void*, const char*, int mode);
    int (*chown)(const void*, const char*, uid_t uid, gid_t gid);
    void (*unlock)(const void*);
    const void * config;
} sysfs_t;


int rootfs_init(const void* cfg);
int rootfs_stat(const void* cfg, const char * path, struct stat * st);
int rootfs_opendir(const void* cfg, void ** handle, const char * path);
int rootfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry);
int rootfs_closedir(const void* cfg, void ** handle);

#define SYSFS_READONLY_ACCESS (S_IRUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH)
#define SYSFS_ALL_ACCESS (0777)

#define SYSFS_MOUNT(mount_loc_name, cfgp, access_mode) { \
    .mount_path = mount_loc_name, \
    .access = access_mode, \
    .mount = rootfs_init, \
    .unmount = SYSFS_NOTSUP, \
    .ismounted = sysfs_always_mounted, \
    .startup = SYSFS_NOTSUP, \
    .mkfs = SYSFS_NOTSUP, \
    .open = SYSFS_NOTSUP, \
    .aio = SYSFS_NOTSUP, \
    .read = SYSFS_NOTSUP, \
    .write = SYSFS_NOTSUP, \
    .ioctl = SYSFS_NOTSUP, \
    .fsync = SYSFS_NOTSUP, \
    .close = SYSFS_NOTSUP, \
    .rename = SYSFS_NOTSUP, \
    .unlink = SYSFS_NOTSUP, \
    .mkdir = SYSFS_NOTSUP, \
    .rmdir = SYSFS_NOTSUP, \
    .remove = SYSFS_NOTSUP, \
    .opendir = rootfs_opendir, \
    .closedir = rootfs_closedir, \
    .readdir_r = rootfs_readdir_r, \
    .link = SYSFS_NOTSUP, \
    .symlink = SYSFS_NOTSUP, \
    .stat = rootfs_stat, \
    .lstat = SYSFS_NOTSUP, \
    .fstat = SYSFS_NOTSUP, \
    .chmod = SYSFS_NOTSUP, \
    .chown = SYSFS_NOTSUP, \
    .unlock = SYSFS_NOTSUP_VOID, \
    .config = cfgp, \
}


#define SYSFS_TERMINATOR { \
    .mount = NULL \
}


extern const sysfs_t sysfs_list[]; //global list of filesystems
const sysfs_t * sysfs_find(const char * path, bool needs_parent);
const char * sysfs_stripmountpath(const sysfs_t * fs, const char * path);

bool sysfs_ispathinvalid(const char * path);
const char * sysfs_getfilename(const char * path, int * elements);
int sysfs_getamode(int flags);
int sysfs_access(int file_mode, int file_uid, int file_gid, int amode);
const char * sysfs_get_filename(const char * path);

static inline bool sysfs_isterminator(const sysfs_t * fs);
bool sysfs_isterminator(const sysfs_t * fs){
    if ( fs->mount == NULL ){
        return true;
    }
    return false;
}

void sysfs_unlock();


extern const char sysfs_validset[];
extern const char sysfs_whitespace[];

int sysfs_file_open(sysfs_file_t * file, const char * name, int mode);
int sysfs_file_ioctl(sysfs_file_t * file, int request, void * ctl);
int sysfs_file_fsync(sysfs_file_t * file);
int sysfs_file_read(sysfs_file_t * file, void * buf, int nbyte);
int sysfs_file_write(sysfs_file_t * file, const void * buf, int nbyte);
int sysfs_file_aio(sysfs_file_t * file, void * aio);
int sysfs_file_close(sysfs_file_t * file);

typedef struct {
    sysfs_file_t file;
    pthread_mutex_t mutex;
} sysfs_shared_state_t;

typedef struct {
    const sysfs_t * devfs;
    const char * name;
    sysfs_shared_state_t * state;
} sysfs_shared_config_t;

int sysfs_shared_open(const sysfs_shared_config_t * config);
int sysfs_shared_ioctl(const sysfs_shared_config_t * config, int request, void * ctl);
int sysfs_shared_fsync(const sysfs_shared_config_t * config);
int sysfs_shared_read(const sysfs_shared_config_t * config, int loc, void * buf, int nbyte);
int sysfs_shared_write(const sysfs_shared_config_t * config, int loc, const void * buf, int nbyte);
int sysfs_shared_aio(const sysfs_shared_config_t * config, void * aio);
int sysfs_shared_close(const sysfs_shared_config_t * config);

typedef struct {
    const void * config;
    void * handle;
    void * buf;
    int flags;
    int loc;
    int nbyte;
	 int result;
} sysfs_read_t;

typedef sysfs_read_t sysfs_write_t;

typedef struct {
    const void * cfg;
    void * handle;
    int request;
    void * ctl;
	 int result;
} sysfs_ioctl_t;

typedef struct {
    struct aiocb * const * list;
    int nent;
    bool block_on_all;
    struct mcu_timeval abs_timeout;
	 int result;
    struct sigevent * event;
} sysfs_aio_suspend_t;

int sysfs_aio_data_transfer_callback(void * context, const mcu_event_t * event);

#endif

#if defined __cplusplus
}
#endif



#endif /* SYSFS_H_ */
