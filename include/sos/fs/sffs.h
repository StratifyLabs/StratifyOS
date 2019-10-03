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


#ifndef SFFS_LITE_H_
#define SFFS_LITE_H_

#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/lock.h>

#include "sos/dev/drive.h"
#include "sos/fs/sysfs.h"


/*
 * # Design
 *
 * This design description describes the filesystem from the bottom up.
 * We start with the physical flash memory, divide it in to blocks, then
 * use those blocks to store file data and list data for keeping track
 * of file and directory content.
 *
 * ## Blocks
 *
 * SFFS assumes the flash memory can be divided into eraseable blocks
 * and writable blocks. Most flash has pages which descibe the smallest
 * eraseable section. The writable block size is defined by SFFS and
 * is larger for better performance.
 *
 * Each block of data can hold one of three types of information.
 *
 * - File Data
 * - File List Data
 * - Serial Number List Data
 *
 * ### The Scratch Pad
 *
 * There is a special section of drive memory that is reserved for
 * the scratch pad. Once every block in the drive has been used, blocks
 * are freed by copying blocks that are still in use to the scratch pad,
 * deleting the flash page, then restoring the data from the scratch pad.
 *
 * This operation is only required for memory that requires flash leveling,
 * but there isn't currently a way to turn it off for SD Cards.
 *
 *
 * ## File Data
 *
 * The file data stores the data for the file which and is associated
 * with the file using the file list data.
 *
 * ## File List Data
 *
 * The file list data includes first a header including
 * the name of the file and a list of blocks that the file
 * is using for data storage. Each entry contains a block number
 * and a segment number. The block number represents the physical location
 * on the memory and the segment number represents the location
 * of the data in the file. *
 * ## Serial Number List Data
 *
 * Everytime a new file is created it is assigned a serial number.
 * There is a single root directly that keeps a list of serial number
 * entries. The entries include
 *
 * - Entry state
 * - Serial number
 * - First list data
 *
 * Each time a file is modified, the state of the serial number
 * is updated. Each time the entry is updated, the entry is marked as dirty
 * and a new entry is appended to the end of the list. This happens
 * in such a way that a power failure at any time cannot corrupt the
 * filesystem.
 *
 * ## Ways to improve performance
 *
 * ### Cache file list location and block
 *
 * Each time the file is read, it looks up the file segment starting
 * at the beginning of the file list. This can take a long time to look
 * up for large files.  The same is true for writing large files.
 *
 * The function to modify is sffs_file_loadsegment(). Add a sffs_list_t data
 * variable to the file handle. When looking for the next segment,
 * always start with the next entry in the list rather than starting over. That
 * can be done using sffs_list_getnext() and passing the current state of the
 * list. Re-init the list if the segment isn't found.
 *
 * ### Cleanup filesystem in the background
 *
 * The scratch pad needs to run periodically (preferably in the background)
 * rather than running during an operation.
 *
 * Create statistical triggers for cleaning the filesystem. As in
 * once, 90% of blocks are dirty, trigger a cleanup.
 *
 * ### Add a compile time switch to disable wear leveling
 *
 * If wear leveling isn't needed, blocks can be erased as
 * soon as they become dirty. There is no need
 * to run a cleanup routine.
 *
 * ### Making blocks the same size as eraseable pages
 *
 * The scratch pad is needed because eraseable pages are usually
 * larger than blocks. If they are the same size, there is no need
 * for the scratch.
 *
 * ### Large drives need a lookup table for the block allocator
 *
 * A table (or list) needs to be implemented for large drives
 * to keep track of which blocks are in use. Maybe just take one bit
 * per block and block off a section of the drive so that
 * blocks can be allocated very fast.
 *
 *
 *
 *
 *
 */

typedef struct {
	sysfs_shared_state_t drive;
	int list_block;
	int serialno_killed;
	int serialno;
	drive_info_t dattr;
} sffs_state_t;

typedef struct {
	sysfs_shared_config_t drive;
} sffs_config_t;


int sffs_init(const void * cfg); //initialize the filesystem
int sffs_mkfs(const void * cfg);

int sffs_opendir(const void * cfg, void ** handle, const char * path);
int sffs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry);
int sffs_closedir(const void * cfg, void ** handle);

int sffs_fstat(const void * cfg, void * handle, struct stat * stat);
int sffs_open(const void * cfg, void ** handle, const char * path, int flags, int mode);
int sffs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte);
int sffs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte);
int sffs_close(const void * cfg, void ** handle);
int sffs_remove(const void * cfg, const char * path);
int sffs_unlink(const void * cfg, const char * path);

void sffs_unlock(const void * cfg);

int sffs_stat(const void * cfg, const char * path, struct stat * stat);

int sffs_unmount(const void * cfg);
int sffs_ismounted(const void * cfg);

#define SFFS_MOUNT(mount_loc_name, cfgp, permissions_value, owner_value) { \
	.mount_path = mount_loc_name, \
	.permissions = permissions_value, \
	.owner = owner_value, \
	.mount = sffs_init, \
	.unmount = sffs_unmount, \
	.ismounted = sffs_ismounted, \
	.startup = SYSFS_NOTSUP, \
	.mkfs = sffs_mkfs, \
	.open = sffs_open, \
	.aio = SYSFS_NOTSUP, \
	.read = sffs_read, \
	.write = sffs_write, \
	.close = sffs_close, \
	.fsync = SYSFS_NOTSUP, \
	.ioctl = SYSFS_NOTSUP, \
	.rename = SYSFS_NOTSUP, \
	.unlink = sffs_unlink, \
	.mkdir = SYSFS_NOTSUP, \
	.rmdir = SYSFS_NOTSUP, \
	.remove = sffs_remove, \
	.opendir = sffs_opendir, \
	.closedir = sffs_closedir, \
	.readdir_r = sffs_readdir_r, \
	.link = SYSFS_NOTSUP, \
	.symlink = SYSFS_NOTSUP, \
	.stat = sffs_stat, \
	.lstat = SYSFS_NOTSUP, \
	.fstat = sffs_fstat, \
	.chmod = SYSFS_NOTSUP, \
	.chown = SYSFS_NOTSUP, \
	.unlock = sffs_unlock, \
	.config = cfgp, \
	}


#endif /* SFFS_LITE_H_ */
