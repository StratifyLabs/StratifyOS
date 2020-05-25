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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SOS_FS_DRIVE_ASSETFS_H_
#define SOS_FS_DRIVE_ASSETFS_H_

#include "mcu/types.h"
#include "sos/dev/drive.h"
#include "../link/types.h"
#include "sysfs.h"

int drive_assetfs_init(const void* cfg);
int drive_assetfs_exit(const void* cfg);
int drive_assetfs_startup(const void* cfg);
int drive_assetfs_open(const void* cfg, void ** handle, const char * path, int flags, int mode);
int drive_assetfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte);
int drive_assetfs_ioctl(const void * cfg, void * handle, int request, void * ctl);
int drive_assetfs_close(const void* cfg, void ** handle);
int drive_assetfs_fstat(const void* cfg, void * handle, struct stat * st);
int drive_assetfs_stat(const void* cfg, const char * path, struct stat * st);
int drive_assetfs_opendir(const void* cfg, void ** handle, const char * path);
int drive_assetfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry);
int drive_assetfs_closedir(const void* cfg, void ** handle);

typedef struct MCU_PACK {
	char name[LINK_NAME_MAX];
	u32 start;
	u32 size;
	u16 uid;
	u16 mode;
} drive_assetfs_dirent_t;


typedef struct {
	u32 count;
	const drive_assetfs_dirent_t entries[];
} drive_assetfs_header_t;


#if !defined __link
typedef struct {
	sysfs_shared_config_t drive;
	u32 offset;
} drive_assetfs_config_t;

typedef struct {
	sysfs_shared_state_t drive;
} drive_assetfs_state_t;


#define DRIVE_ASSETFS_MOUNT(mount_loc_name, cfgp, permissions_value, owner_value) { \
	.mount_path = mount_loc_name, \
	.permissions = permissions_value, \
	.owner = owner_value, \
	.mount = drive_assetfs_init, \
	.unmount = drive_assetfs_exit, \
	.ismounted = sysfs_always_mounted, \
	.startup = drive_assetfs_startup, \
	.mkfs = SYSFS_NOTSUP, \
	.open = drive_assetfs_open, \
	.aio = SYSFS_NOTSUP, \
	.read = drive_assetfs_read, \
	.write = SYSFS_NOTSUP, \
	.close = drive_assetfs_close, \
	.ioctl = drive_assetfs_ioctl, \
	.rename = SYSFS_NOTSUP, \
	.fsync = SYSFS_NOTSUP, \
	.unlink = SYSFS_NOTSUP, \
	.mkdir = SYSFS_NOTSUP, \
	.rmdir = SYSFS_NOTSUP, \
	.remove = SYSFS_NOTSUP, \
	.opendir = drive_assetfs_opendir, \
	.closedir = drive_assetfs_closedir, \
	.readdir_r = drive_assetfs_readdir_r, \
	.link = SYSFS_NOTSUP, \
	.symlink = SYSFS_NOTSUP, \
	.stat = drive_assetfs_stat, \
	.lstat = SYSFS_NOTSUP, \
	.fstat = drive_assetfs_fstat, \
	.chmod = SYSFS_NOTSUP, \
	.chown = SYSFS_NOTSUP, \
	.unlock = SYSFS_NOTSUP_VOID, \
	.config = cfgp, \
	}

#endif


#endif /* SOS_FS_DRIVE_ASSETFS_H_ */
