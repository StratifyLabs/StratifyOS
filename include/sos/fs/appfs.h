// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_FS_APPFS_H_
#define SOS_FS_APPFS_H_

#include "types.h"

int appfs_init(const void* cfg);
int appfs_startup(const void* cfg);
int appfs_mkfs(const void* cfg);
int appfs_open(const void* cfg, void ** handle, const char * path, int flags, int mode);
int appfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte);
int appfs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte);
//int appfs_read_async(const void* cfg, void * handle, devfs_async_t * op);
//int appfs_write_async(const void* cfg, void * handle, devfs_async_t * op);
int appfs_ioctl(const void * cfg, void * handle, int request, void * ctl);
int appfs_close(const void* cfg, void ** handle);
int appfs_fstat(const void* cfg, void * handle, struct stat * st);
int appfs_stat(const void* cfg, const char * path, struct stat * st);
int appfs_opendir(const void* cfg, void ** handle, const char * path);
int appfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry);
int appfs_closedir(const void* cfg, void ** handle);
int appfs_unlink(const void* cfg, const char * path);


#define APPFS_MOUNT(mount_loc_name, cfgp, permissions_value, owner_value) { \
	.mount_path = mount_loc_name, \
	.permissions = permissions_value, \
	.owner = owner_value, \
	.mount = appfs_init, \
	.unmount = SYSFS_NOTSUP, \
	.ismounted = sysfs_always_mounted, \
	.startup = appfs_startup, \
	.mkfs = appfs_mkfs, \
	.open = appfs_open, \
	.aio = SYSFS_NOTSUP, \
	.read = appfs_read, \
	.write = appfs_write, \
	.close = appfs_close, \
	.ioctl = appfs_ioctl, \
	.rename = SYSFS_NOTSUP, \
	.fsync = SYSFS_NOTSUP, \
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
	.config = cfgp, \
	}


#endif /* SOS_FS_APPFS_H_ */
