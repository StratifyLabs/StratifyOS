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

#ifdef __SIM__
#include "sim_device.h"
#else
#include "iface/device_config.h"
#endif

int sysfs_notsup(void);
void * sysfs_notsup_null(void);
void sysfs_notsup_void(void);
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
	int (*priv_read)(const void*, void*, device_transfer_t*);
	int (*priv_write)(const void*, void*, device_transfer_t*);
	int (*priv_ioctl)(const void*, void*, int, void*);
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

int devfs_init(const void * cfg);
int devfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode);
int devfs_priv_read(const void * cfg, void * handle, device_transfer_t * op);
int devfs_priv_write(const void * cfg, void * handle, device_transfer_t * op);
int devfs_priv_ioctl(const void * cfg, void * handle, int request, void * ctl);
int devfs_close(const void * cfg, void ** handle);
int devfs_fstat(const void * cfg, void * handle, struct stat * st);
int devfs_stat(const void * cfg, const char * path, struct stat * st);
int devfs_opendir(const void * cfg, void ** handle, const char * path);
int devfs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry);
int devfs_closedir(const void * cfg, void ** handle);

int appfs_init(const void* cfg);
int appfs_startup(const void* cfg);
int appfs_mkfs(const void* cfg);
int appfs_open(const void* cfg, void ** handle, const char * path, int flags, int mode);
int appfs_priv_read(const void* cfg, void * handle, device_transfer_t * op);
int appfs_priv_write(const void* cfg, void * handle, device_transfer_t * op);
int appfs_priv_ioctl(const void * cfg, void * handle, int request, void * ctl);
int appfs_close(const void* cfg, void ** handle);
int appfs_fstat(const void* cfg, void * handle, struct stat * st);
int appfs_stat(const void* cfg, const char * path, struct stat * st);
int appfs_opendir(const void* cfg, void ** handle, const char * path);
int appfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry);
int appfs_closedir(const void* cfg, void ** handle);
int appfs_unlink(const void* cfg, const char * path);

#define SYSFS_READONLY_ACCESS (S_IRUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH)
#define SYSFS_ALL_ACCESS (0777)

#define SYSFS_ROOT(mount_loc_name, cfgp, access_mode) { \
		.mount_path = mount_loc_name, \
		.access = access_mode, \
		.mount = rootfs_init, \
		.unmount = SYSFS_NOTSUP, \
		.ismounted = sysfs_always_mounted, \
		.startup = SYSFS_NOTSUP, \
		.mkfs = SYSFS_NOTSUP, \
		.open = SYSFS_NOTSUP, \
		.priv_read = NULL, \
		.priv_write = NULL, \
		.read = SYSFS_NOTSUP, \
		.write = SYSFS_NOTSUP, \
		.priv_ioctl = SYSFS_NOTSUP, \
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

#define SYSFS_DEV(mount_loc_name, cfgp, access_mode) { \
		.mount_path = mount_loc_name, \
		.access = access_mode, \
		.mount = devfs_init, \
		.unmount = SYSFS_NOTSUP, \
		.ismounted = sysfs_always_mounted, \
		.startup = SYSFS_NOTSUP, \
		.mkfs = SYSFS_NOTSUP, \
		.open = devfs_open, \
		.priv_read = devfs_priv_read, \
		.priv_write = devfs_priv_write, \
		.priv_ioctl = devfs_priv_ioctl, \
		.read = NULL, \
		.write = NULL, \
		.close = devfs_close, \
		.rename = SYSFS_NOTSUP, \
		.unlink = SYSFS_NOTSUP, \
		.mkdir = SYSFS_NOTSUP, \
		.rmdir = SYSFS_NOTSUP, \
		.remove = SYSFS_NOTSUP, \
		.opendir = devfs_opendir, \
		.closedir = devfs_closedir, \
		.readdir_r = devfs_readdir_r, \
		.link = SYSFS_NOTSUP, \
		.symlink = SYSFS_NOTSUP, \
		.stat = devfs_stat, \
		.lstat = SYSFS_NOTSUP, \
		.fstat = devfs_fstat, \
		.chmod = SYSFS_NOTSUP, \
		.chown = SYSFS_NOTSUP, \
		.unlock = SYSFS_NOTSUP_VOID, \
		.cfg = cfgp, \
}

#define SYSFS_APP(mount_loc_name, cfgp, access_mode) { \
		.mount_path = mount_loc_name, \
		.access = access_mode, \
		.mount = appfs_init, \
		.unmount = SYSFS_NOTSUP, \
		.ismounted = sysfs_always_mounted, \
		.startup = appfs_startup, \
		.mkfs = appfs_mkfs, \
		.open = appfs_open, \
		.priv_read = appfs_priv_read, \
		.priv_write = appfs_priv_write, \
		.read = NULL, \
		.write = NULL, \
		.close = appfs_close, \
		.priv_ioctl = appfs_priv_ioctl, \
		.rename = SYSFS_NOTSUP, \
		.unlink = appfs_unlink, \
		.mkdir = SYSFS_NOTSUP, \
		.rmdir = SYSFS_NOTSUP, \
		.remove = SYSFS_NOTSUP, \
		.opendir = appfs_opendir, \
		.closedir = appfs_closedir, \
		.readdir_r = appfs_readdir_r, \
		.link = SYSFS_NOTSUP, \
		.symlink = SYSFS_NOTSUP, \
		.stat = appfs_stat, \
		.lstat = SYSFS_NOTSUP, \
		.fstat = appfs_fstat, \
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

void sysfs_unlock(void);


extern const char sysfs_validset[];
extern const char sysfs_whitespace[];


#endif /* SYSFS_H_ */
