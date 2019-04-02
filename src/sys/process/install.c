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

/*! \addtogroup SYSCALLS_PROCESS
 * @{
 */

/*! \file */

#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "sos/fs/devfs.h"
#include "cortexm/task.h"
#include "mcu/debug.h"
#include "sos/fs/sysfs.h"
#include "../scheduler/scheduler_local.h"
#include "../sysfs/appfs_local.h"

static u8 launch_count = 0;

int install(const char * path,
				char * exec_path,
				int options,
				int ram_size,
				int (*update_progress)(const void *, int, int),
				const void * update_context){

	int install_fd;
	int image_fd;
	int bytes_read;
	int bytes_cumm;
	appfs_file_t * hdr;
	appfs_installattr_t attr;
	struct stat st;
	char name[NAME_MAX];
	char target_path[PATH_MAX];
	int len;
	int needs_install = 0;

	if( stat(path, &st) < 0 ){
		mcu_debug_log_error(MCU_DEBUG_SYS, "Can't find path %s", path);
		return -1;
	}

	strncpy(name, sysfs_getfilename(path,0), NAME_MAX-1);
	if( options & APPFS_FLAG_IS_FLASH ){
		strncpy(target_path, "/app/flash/", PATH_MAX-1);
	} else {
		strncpy(target_path, "/app/ram/", PATH_MAX-1);
	}
	strncat(target_path, name, PATH_MAX-1);


	//does the image already exist at the target location
	if( access(target_path, R_OK) == 0 ){

		if( options & APPFS_FLAG_IS_REPLACE ){ //need to replace the target image
			unlink(target_path);
			needs_install = 1;
		} else if( options & APPFS_FLAG_IS_UNIQUE ){ //need to run another copy of the target image
			needs_install = 1;
		}

	} else {
		needs_install = 1; //target image does not exist and needs to be installed
	}

	if( needs_install ){

		//first install the file using appfs/.install
		image_fd = open(path, O_RDONLY);
		if( image_fd < 0 ){
			mcu_debug_log_error(MCU_DEBUG_SYS, "Failed to open %s (%d)", path, errno);
			return -1;
		}

		install_fd = open("/app/.install", O_WRONLY);
		if( install_fd < 0 ){
			close(image_fd);
			mcu_debug_log_error(MCU_DEBUG_SYS, "Failed to open /app/.install %d", errno);
			return -1;
		}

		attr.loc = 0;
		bytes_cumm = 0;

		do {
			//check the image
			memset(attr.buffer, 0xFF, APPFS_PAGE_SIZE);
			bytes_read = read(image_fd, attr.buffer, APPFS_PAGE_SIZE);
			if( bytes_read > 0 ){
				attr.nbyte = bytes_read;
				bytes_cumm += attr.nbyte;

				if( attr.loc == 0 ){
					hdr = (appfs_file_t*)attr.buffer;
					//update the header for the image to be installed
					if( options & APPFS_FLAG_IS_UNIQUE ){
						strncpy(hdr->hdr.name, name, NAME_MAX-1);
						len = strnlen(hdr->hdr.name, NAME_MAX-2);
						hdr->hdr.name[len] = (launch_count % 10) + '0';
						hdr->hdr.name[len+1] = 0;
						launch_count++;
					} else {
						strncpy(hdr->hdr.name, name, NAME_MAX-1);
					}
					hdr->exec.o_flags = options;
					if( ram_size > 0 ){
						hdr->exec.ram_size = ram_size;
					}
				}

				if( ioctl(install_fd, I_APPFS_INSTALL, &attr) < 0 ){
					close(image_fd);
					close(install_fd);
					return -1;
				}

				if( update_progress != 0 ){
					if( update_progress(update_context, bytes_cumm, st.st_size) < 0 ){
						//aborted by caller
						close(image_fd);
						close(install_fd);
						return -1;
					}
				}

				attr.loc += APPFS_PAGE_SIZE;
			}

		} while( bytes_read == APPFS_PAGE_SIZE );
		close(image_fd);
		close(install_fd);
	}


	if( exec_path ){
		strncpy(exec_path, target_path, PATH_MAX-1);
	}

	return 0;
}

