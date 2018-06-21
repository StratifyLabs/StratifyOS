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

#ifndef DRIVE_SDSPI_LOCAL_H_
#define DRIVE_SDSPI_LOCAL_H_

#include "device/drive_sdspi.h"

#define SDSPI_IOC_IDENT_CHAR 't'


#define SDSPI_START_BLOCK_TOKEN 0xFE
#define SDSPI_START_BLOCK_WRITE_MULTIPLE_TOKEN 0xFC
#define SDSPI_STOP_TRAN_TOKEN 0xFB

#define SDSPI_CMD0_GO_IDLE_STATE 0
#define SDSPI_CMD1_SEND_OP_COND 1
#define SDSPI_CMD8_SEND_IF_COND 8
#define SDSPI_CMD9_SEND_CSD 9
#define SDSPI_CMD10_SEND_CID 10
#define SDSPI_CMD12_STOP_TRANSMISSION 12
#define SDSPI_CMD13_SD_STATUS 13
#define SDSPI_CMD16_SET_BLOCKLEN 16
#define SDSPI_CMD17_READ_SINGLE_BLOCK 17
#define SDSPI_CMD18_READ_MULTIPLE_BLOCK 18
#define SDSPI_CMD24_WRITE_SINGLE_BLOCK 24
#define SDSPI_CMD25_WRITE_MULTIPLE_BLOCK 25
#define SDSPI_CMD28_SET_WRITE_PROT 28
#define SDSPI_CMD32_ERASE_WR_BLK_START 32
#define SDSPI_CMD33_ERASE_WR_BLK_END 33
#define SDSPI_CMD38_ERASE 38


#define SDSPI_ACMD41_SD_SEND_OP_COND 41

#define SDSPI_CMD55_APP_CMD 55
#define SDSPI_CMD58_READ_OCR 58
#define SDSPI_CMD59_CRC_ON_OFF 59

typedef union {
	struct MCU_PACK {
		unsigned idle:1;
		unsigned erase_reset:1;
		unsigned illegal_command:1;
		unsigned crc_error:1;
		unsigned erase_sequence_error:1;
		unsigned addr_error:1;
		unsigned param_error:1;
		unsigned start:1;
	};
	uint8_t u8;
} drive_sdspi_r1_t;

typedef union {
	struct MCU_PACK {
		drive_sdspi_r1_t r1;
		unsigned card_locked:1;
		unsigned wp_erase_skip_lock_unlock_failed:1;
		unsigned unspecified_error:1;
		unsigned controller_error:1;
		unsigned ecc_failed:1;
		unsigned wp_violation:1;
		unsigned erase_parameter:1;
		unsigned out_of_range:1;
	};
	uint16_t u16;
	uint8_t u8[2];
} drive_sdspi_r2_t;

typedef union {
	struct MCU_PACK {
		drive_sdspi_r1_t r1;
		uint8_t arg[4];
	};
	uint8_t u8[5];
} drive_sdspi_r3_t;

//seven is the same as three
typedef drive_sdspi_r3_t drive_sdspi_r7_t;

typedef union {
	drive_sdspi_r1_t r1;
	drive_sdspi_r2_t r2;
	drive_sdspi_r3_t r3;
	uint8_t u8[5];
} drive_sdspi_r_t;

typedef union {
	uint64_t u64[2];
	uint32_t u32[4];
	uint16_t u16[8];
	uint8_t u8[16];
} drive_sdspi_csd_t;


typedef union {
	uint64_t u64[8];
	uint32_t u32[16];
	uint16_t u16[32];
	uint8_t u8[64];
} _drive_sdspi_status_t;


typedef struct MCU_PACK {
	uint32_t size;
} drive_sdspi_attr_t;


#define I_SDSPI_TOTAL 2


#endif /* DRIVE_SDSPI_LOCAL_H_ */
