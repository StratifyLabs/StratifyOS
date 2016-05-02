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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef APPFS_H_
#define APPFS_H_

#include <stdbool.h>
#include <mcu/types.h>

#define APPFS_EXEC_OPTIONS_FLASH (1<<0)
#define APPFS_EXEC_OPTIONS_RAM (0<<0)
#define APPFS_EXEC_OPTIONS_STARTUP (1<<1) //if set executes on boot

typedef struct {
	void (*startup)(char * path_arg) /*! The startup routine */;
	void * code_start /*! The start of memory */;
	u32 code_size /*! The size of the code section */;
	void * ram_start /*! The start of memory */;
	i32 ram_size /*! The size of  memory (total RAM for process)*/;
	i32 data_size /*! Number of initialized bytes */;
	u32 options /*! must be valid to execute the file */;
	u32 signature /*! The kernel signature used for the build */;
} appfs_exec_t; //32 bytes

typedef struct {
	char name[NAME_MAX] /*! The name of the process or file (must be specially written to the binary) */;
	mode_t mode;
} appfs_hdr_t;  //28 bytes

typedef struct {
	appfs_hdr_t hdr;
	appfs_exec_t exec;
} appfs_file_t;

bool appfs_util_isexecutable(const appfs_file_t * info);


#endif /* APPFS_H_ */
