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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#ifndef IAP_H_
#define IAP_H_



#define IAP_LOCATION 0x1FFF1FF1


typedef union {
	struct __attribute__((packed)){
		unsigned cmd;
		unsigned start;
		unsigned end;
		unsigned cpu_freq /*! Cpu freq in KHz */;
		unsigned dummy;
	} sector;
	struct __attribute__((packed)){
		unsigned cmd;
		unsigned dest;
		unsigned src;
		unsigned size;
		unsigned cpu_freq /*! Clock in Khz */;
	} mem;
} iap_op_t;


typedef struct __attribute__((packed)){
	unsigned int status;
	unsigned int dummy[4];
} iap_result_t;

enum {
	IAP_CMD_PREPARE_SECTOR = 50,
	IAP_CMD_CPY_RAM_TO_FLASH,
	IAP_CMD_ERASE_SECTOR,
	IAP_CMD_BLANK_CHECK_SECTOR,
	IAP_CMD_RD_PART_ID,
	IAP_CMD_RD_BOOT_CODE_VERSION,
	IAP_CMD_CMP_ADDR,
	IAP_CMD_INVOKE_ISP,
	IAP_CMD_RD_SERIAL_NUMBER
};

enum {
	IAP_RESULT_CMD_SUCCESS,
	IAP_RESULT_INVALID_COMMAND,
	IAP_RESULT_SRC_ADDR_ERROR,
	IAP_RESULT_DST_ADDR_ERROR,
	IAP_RESULT_SRC_ADDR_NOT_MAPPED,
	IAP_RESULT_DST_ADDR_NOT_MAPPED,
	IAP_RESULT_COUNT_ERROR,
	IAP_RESULT_INVALID_SECTOR,
	IAP_RESULT_SECTOR_NOT_BLANK,
	IAP_RESULT_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
	IAP_RESULT_COMPARE_ERROR,
	IAP_RESULT_BUSY
};


//Function is provided in ROM by linker
void mcu_iap_entry(unsigned param_tab[],unsigned result_tab[]);


#endif /* IAP_H_ */
