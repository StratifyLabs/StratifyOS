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

#define APPFS_VERSION (0x030000)


#include "sos/link/types.h"
#define APPFS_RAM_USAGE_WORDS(x) ((x*2+31)/32)
#define APPFS_IOC_IDENT_CHAR 'a'

enum appfs_flags {
	APPFS_FLAG_IS_FLASH /*! Application is stored in flash */ = (1<<0),
	APPFS_FLAG_IS_STARTUP /*! Application stored in flash runs at startup */ = (1<<1),
	APPFS_FLAG_IS_ROOT /*! Application runs as root */ = (1<<3),
	APPFS_FLAG_IS_REPLACE /*! Application will replace an existing application (default is to duplicate) */ = (1<<4),
	APPFS_FLAG_IS_ORPHAN /*! Application runs as an orphan (parent doesn't need to wait()). */ = (1<<5),
	APPFS_FLAG_IS_UNIQUE /*! Application will run with a unique name by appending a counter value */ = (1<<6),
	APPFS_FLAG_IS_CODE_TIGHTLY_COUPLED /*! Application code will be installed in tightly coupled memory if possible */ = (1<<7),
	APPFS_FLAG_IS_DATA_TIGHTLY_COUPLED /*! Application data will reside in tightly coupled memory if possible */ = (1<<8),
	APPFS_FLAG_IS_CODE_EXTERNAL /*! Application code will be installed in external memory if possible */ = (1<<9),
	APPFS_FLAG_IS_DATA_EXTERNAL /*! Application code will reside in external memory if possible */ = (1<<10),
	APPFS_FLAG_IS_HASHED /*! Application binary has SHA256 hash appended to the end */ = (1<<11),
};

#define APPFS_PAGE_SIZE 256
typedef struct {
	u32 loc;
	u32 nbyte;
	u8 buffer[APPFS_PAGE_SIZE];
} appfs_installattr_t;

typedef appfs_installattr_t appfs_createattr_t;

typedef struct MCU_PACK {
	u16 mode;
	u16 version;
	u32 ram_size;
	u32 o_flags;
	u32 signature;
	u8 id[LINK_NAME_MAX];
	u8 name[LINK_NAME_MAX];
	u32 resd[8];
} appfs_info_t;


#define I_APPFS_GETVERSION _IOCTL(APPFS_IOC_IDENT_CHAR, I_MCU_GETVERSION)


//Install an executable in RAM or Flash
#define I_APPFS_INSTALL _IOCTLW(APPFS_IOC_IDENT_CHAR, 1, appfs_installattr_t)

//Create a data file in RAM or flash
#define I_APPFS_CREATE _IOCTLW(APPFS_IOC_IDENT_CHAR, 2, appfs_createattr_t)
#define I_APPFS_FREE_RAM _IOCTL(APPFS_IOC_IDENT_CHAR, 3)
#define I_APPFS_RECLAIM_RAM _IOCTL(APPFS_IOC_IDENT_CHAR, 4)
#define I_APPFS_GETINFO _IOCTLR(APPFS_IOC_IDENT_CHAR,5,appfs_info_t)

#define APPFS_CREATE_SIGNATURE 0x12345678



typedef struct MCU_PACK {
	u32 startup /*! The startup routine */;
	u32 code_start /*! The start of memory */;
	u32 code_size /*! The size of the code section */;
	u32 ram_start /*! The start of memory */;
	u32 ram_size /*! The size of  memory (total RAM for process) */;
	u32 data_size /*! Size of "data" section */;
	u32 o_flags /*! Installation options */;
	u32 signature /*! must be valid to execute the file */;
} appfs_exec_t; //32 bytes

typedef struct MCU_PACK {
	char name[LINK_NAME_MAX] /*! The name of the process or file (must be specially written to the binary) */;
	char id[LINK_NAME_MAX];
	u16 mode /*! Access mode */;
	u16 version /*! BCD 0xMMmm version (e.g, 1.2 is 0x0102) */;
} appfs_header_t;  //28 + 24 bytes

typedef struct MCU_PACK {
	appfs_header_t hdr;
	appfs_exec_t exec;
} appfs_file_t;



#endif /* SOS_DEV_APPFS_H_ */
