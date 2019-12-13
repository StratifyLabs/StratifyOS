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

#ifndef SOS_FS_ASSETFS_H_
#define SOS_FS_ASSETFS_H_

#include "mcu/types.h"

int assetfs_init(const void* cfg);
int assetfs_startup(const void* cfg);
int assetfs_open(const void* cfg, void ** handle, const char * path, int flags, int mode);
int assetfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte);
int assetfs_ioctl(const void * cfg, void * handle, int request, void * ctl);
int assetfs_close(const void* cfg, void ** handle);
int assetfs_fstat(const void* cfg, void * handle, struct stat * st);
int assetfs_stat(const void* cfg, const char * path, struct stat * st);
int assetfs_opendir(const void* cfg, void ** handle, const char * path);
int assetfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry);
int assetfs_closedir(const void* cfg, void ** handle);

#define ASSETFS_FILE(name, file) \
	__asm__(".section .rodata\n" \
	".global assetfs_incbin_" #name "_start\n" \
	".type assetfs_incbin_" #name "_start, %object\n" \
	".balign 16\n" \
	"assetfs_incbin_" #name "_start:\n" \
	".incbin \"" file "\"\n" \
	\
	".global assetfs_incbin_" #name "_end\n" \
	".type assetfs_incbin_" #name "_end, %object\n" \
	".balign 1\n" \
	"assetfs_incbin_" #name "_end:\n" \
	".byte 0\n" \
	); \
	extern const __attribute__((aligned(16))) void* assetfs_incbin_##name##_start; \
	extern const void* assetfs_incbin_##name##_end

#define ASSETFS_START(name) (&assetfs_incbin_##name##_start)
#define ASSETFS_END(name) (&assetfs_incbin_##name##_end)

#define ASSETFS_ENTRY(file_name, object_name, mode_value, uid_value) \
{ .name = file_name, .start = ASSETFS_START(object_name), .end = ASSETFS_END(object_name), .mode = mode_value, .uid = uid_value }

typedef struct {
	char name[NAME_MAX];
	const void * start;
	const void * end;
	u16 uid;
	u16 mode;
} assetfs_dirent_t;

typedef struct {
	u32 count;
	const assetfs_dirent_t entries[];
} assetfs_config_t;


#define ASSETFS_MOUNT(mount_loc_name, cfgp, permissions_value, owner_value) { \
	.mount_path = mount_loc_name, \
	.permissions = permissions_value, \
	.owner = owner_value, \
	.mount = assetfs_init, \
	.unmount = SYSFS_NOTSUP, \
	.ismounted = sysfs_always_mounted, \
	.startup = assetfs_startup, \
	.mkfs = SYSFS_NOTSUP, \
	.open = assetfs_open, \
	.aio = SYSFS_NOTSUP, \
	.read = assetfs_read, \
	.write = SYSFS_NOTSUP, \
	.close = assetfs_close, \
	.ioctl = assetfs_ioctl, \
	.rename = SYSFS_NOTSUP, \
	.fsync = SYSFS_NOTSUP, \
	.unlink = SYSFS_NOTSUP, \
	.mkdir = SYSFS_NOTSUP, \
	.rmdir = SYSFS_NOTSUP, \
	.remove = SYSFS_NOTSUP, \
	.opendir = assetfs_opendir, \
	.closedir = assetfs_closedir, \
	.readdir_r = assetfs_readdir_r, \
	.link = SYSFS_NOTSUP, \
	.symlink = SYSFS_NOTSUP, \
	.stat = assetfs_stat, \
	.lstat = SYSFS_NOTSUP, \
	.fstat = assetfs_fstat, \
	.chmod = SYSFS_NOTSUP, \
	.chown = SYSFS_NOTSUP, \
	.unlock = SYSFS_NOTSUP_VOID, \
	.config = cfgp, \
	}


#endif /* SOS_FS_ASSETFS_H_ */
