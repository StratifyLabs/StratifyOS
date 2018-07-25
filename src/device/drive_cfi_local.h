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

#ifndef DRIVE_CFI_LOCAL_H_
#define DRIVE_CFI_LOCAL_H_

#include "device/drive_cfi.h"


enum {
    CFI_COMMAND_WRITE_ENABLE = 0x06,
    CFI_COMMAND_WRITE_DISABLE = 0x04,

    CFI_COMMAND_VOLATILE_SR_WRITE_ENABLE = 0x50,
    CFI_COMMAND_NOP = 0x00,
    CFI_COMMAND_NO_OPERATION = 0x00,
    CFI_COMMAND_MANUFACTURER_DEVICE_ID = 0x90,
    CFI_COMMAND_JEDEC_ID = 0x9f,
    CFI_COMMAND_QUAD_JEDEC_ID = 0xaf,
    CFI_COMMAND_READ_UNIQUE_ID = 0x4b,
    CFI_COMMAND_READ = 0x03,
    CFI_COMMAND_FAST_READ = 0x0b,
    CFI_COMMAND_PAGE_PROGRAM = 0x02,
    CFI_COMMAND_SECTOR_ERASE= 0x20,
    CFI_COMMAND_BLOCK_ERASE_32KB = 0x52, //non-standard?
    CFI_COMMAND_BLOCK_ERASE_64KB = 0xd8,
    CFI_COMMAND_CHIP_ERASE = 0xc7,
    CFI_COMMAND_READ_STATUS = 0x5,
    CFI_COMMAND_WRITE_STATUS = 0x1,

    CFI_COMMAND_READ_SFDP = 0x5a,


    CFI_COMMAND_GLOBAL_BLOCK_LOCK = 0x7e,
    CFI_COMMAND_GLOBAL_BLOCK_UNLOCK = 0x98,
    CFI_COMMAND_ENABLE_RESET = 0x66,
    CFI_COMMAND_RESET_DEVICE = 0x99,



    //non-standard?
    CFI_COMMAND_READ_STATUS2 = 0x35,
    CFI_COMMAND_WRITE_STATUS2 = 0x31,
    CFI_COMMAND_READ_STATUS3 = 0x15,
    CFI_COMMAND_WRITE_STATUS3 = 0x11,
    CFI_COMMAND_RELEASE_POWER_DOWN = 0xab,



    CFI_COMMAND_SUSPEND_PROGRAM_ERASE = 0xb0,
    CFI_COMMAND_RESUME_PROGRAM_ERASE = 0x30,
    CFI_COMMAND_READ_BLOCK_PROTECTION = 0x72,
    CFI_COMMAND_WRITE_BLOCK_PROTECTION = 0x42,

    //Quad and dual only commands
    CFI_COMMAND_ENABLE_QUAD_IO = 0x38, //switch from SPI to quad mode
    CFI_COMMAND_RESET_QUAD_IO = 0xff, //resets from QUAD back to SPI
    CFI_COMMAND_QUAD_PAGE_PROGRAM = 0x32,
    CFI_COMMAND_DUAL_FAST_READ = 0x3b,
    CFI_COMMAND_QUAD_FAST_READ = 0xeb,
    CFI_COMMAND_QUAD_CHIP_ERASE = 0x60
};


enum {
    CFI_STATUS_STATUS_IS_BUSY = (1<<0),
    CFI_STATUS_STATUS_IS_WRITE_ENABLE = (1<<1)
};

enum {
    CFI_SFDP_SIGNATURE = 0x50444653
};


typedef struct MCU_PACK {
    u32 signature; //"SFDP"
    u16 revision;
    u8 header_count;
    u8 resd;
} cfi_sfdp_header_t;

typedef struct MCU_PACK {
    u8 jedec_id;
    u16 revision;
    u8 length;
    u32 table_pointer; //top 8 bits are ignored - must be cleared
} cfi_sfdp_parameter_t;


enum {
    CFI_SFDP_FLAG_IS_FAST_READ_114 = (1<<0),
    CFI_SFDP_FLAG_IS_FAST_READ_144 = (1<<1),
    CFI_SFDP_FLAG_IS_FAST_READ_122 = (1<<2),
    CFI_SFDP_FLAG_IS_DOUBLE_TRASFER_RATE = (1<<3),
    CFI_SFDP_FLAG_IS_THREE_BYTE_ADDRESSING = (1<<4),
    CFI_SFDP_FLAG_IS_FOUR_BYTE_ADDRESSING = (1<<5),
    CFI_SFDP_FLAG_IS_FOUR_BYTE_ADDRESSING_COMMAND = (1<<6),
    CFI_SFDP_FLAG_IS_FAST_READ_112 = (1<<7),
    CFI_SFDP_FLAG_IS_WRITE_GRANULARITY = (1<<8),
    CFI_SFDP_FLAG_IS_FAST_READ_444 = (1<<9),
    CFI_SFDP_FLAG_IS_FAST_READ_222 = (1<<10)
};




#endif /* DRIVE_CFI_LOCAL_H_ */
