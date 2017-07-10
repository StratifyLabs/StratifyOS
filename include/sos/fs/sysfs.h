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


#ifndef SYSFS_H_
#define SYSFS_H_

#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/lock.h>

#ifdef __SIM__
#include "sim_device.h"
#else
#include "sos/fs/devfs.h"
#endif

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
	int (*read_async)(const void*, void*, devfs_async_t*); //must be called in priv mode
	int (*write_async)(const void*, void*, devfs_async_t*); //must be called in priv mode
	int (*ioctl)(const void*, void*, int, void*); //must be called in priv mode
	int (*read)(const void*, void*, int, int, void*, int);
	int (*write)(const void*, void*, int, int, const void*, int);
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
	const void * cfg;
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
		.read_async = NULL, \
		.write_async = NULL, \
		.read = SYSFS_NOTSUP, \
		.write = SYSFS_NOTSUP, \
		.ioctl = SYSFS_NOTSUP, \
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
		.cfg = cfgp, \
}


#define SYSFS_TERMINATOR { \
	.mount = NULL \
}


extern const sysfs_t const sysfs_list[]; //global list of filesystems
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

int u_open(open_file_t * open_file, const char * name);
int u_ioctl(open_file_t * open_file, int request, void * ctl);
int u_read(open_file_t * open_file, void * buf, int nbyte);
int u_write(open_file_t * open_file, const void * buf, int nbyte);
int u_close(open_file_t * open_file);


#endif /* SYSFS_H_ */
