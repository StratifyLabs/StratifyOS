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

#ifndef SOS_DEV_APPFS_H_
#define SOS_DEV_APPFS_H_

#include "sos/link/types.h"

#define APPFS_PAGE_SIZE 256
typedef struct {
	u32 loc;
	u32 nbyte;
	u8 buffer[APPFS_PAGE_SIZE];
} appfs_installattr_t;

typedef appfs_installattr_t appfs_createattr_t;

//Install an executable in RAM or Flash
#define I_APPFS_INSTALL _IOCTLW('a', 0, appfs_installattr_t)

//Create a data file in RAM or flash
#define I_APPFS_CREATE _IOCTLW('a', 1, appfs_createattr_t)
#define I_APPFS_FREE_RAM _IOCTL('a', 2)
#define I_APPFS_RECLAIM_RAM _IOCTL('a', 3)

#define APPFS_CREATE_SIGNATURE 0x12345678

#define LINK_APPFS_EXEC_OPTIONS_FLASH (1<<0)
#define LINK_APPFS_EXEC_OPTIONS_STARTUP (1<<1) //if set executes on boot
#define LINK_APPFS_EXEC_OPTIONS_ROOT (1<<3) //run as root
#define LINK_APPFS_EXEC_OPTIONS_REPLACE (1<<4) //replace (default is to duplicate)
#define LINK_APPFS_EXEC_OPTIONS_ORPHAN (1<<5) //calling process wont' be parent
#define LINK_APPFS_EXEC_OPTIONS_UNIQUE (1<<6) //install with a unique name in the flash or RAM

typedef struct {
	u32 startup /*! The startup routine */;
	u32 code_start /*! The start of memory */;
	u32 code_size /*! The size of the code section */;
	u32 ram_start /*! The start of memory */;
	u32 ram_size /*! The size of  memory (total RAM for process) */;
	u32 data_size /*! Size of "data" section */;
	u32 options /*! A pointer to the re-entrancy structure */;
	u32 signature /*! must be valid to execute the file */;
} link_appfs_exec_t; //32 bytes

typedef struct {
	char name[LINK_NAME_MAX] /*! The name of the process or file (must be specially written to the binary) */;
	u32 mode;
} link_appfs_hdr_t;  //28 bytes

typedef struct {
	link_appfs_hdr_t hdr;
	link_appfs_exec_t exec;
} link_appfs_file_t;



#endif /* SOS_DEV_APPFS_H_ */
