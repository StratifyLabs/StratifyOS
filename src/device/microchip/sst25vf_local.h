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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef SST25VF_LOCAL_H_
#define SST25VF_LOCAL_H_

#include <errno.h>
#include <unistd.h>
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/debug.h"
#include "device/microchip/sst25vf.h"

#define SST25VF_CHIP_ERASE_TIME (75*1000)
#define SST25VF_BLOCK_ERASE_TIME (30*1000)
#define SST25VF_BLOCK_ERASE_SIZE 4096
#define SST25VF_BLOCK_SIZE 128


#define SST25VF_INS_RD 0x03
#define SST25VF_INS_RD_HS 0x0B
#define SST25VF_INS_BLOCK_ERASE_4KB 0x20
#define SST25VF_INS_BLOCK_ERASE_32KB 0x52
#define SST25VF_INS_BLOCK_ERASE_64KB 0xD8
#define SST25VF_INS_CHIP_ERASE 0x60
#define SST25VF_INS_PROGRAM 0x02
#define SST25VF_INS_SEQ_PROGRAM 0xAD
#define SST25VF_INS_WRITE_ENABLE 0x06
#define SST25VF_INS_WRITE_DISABLE 0x04
#define SST25VF_INS_WR_STATUS_ENABLE 0x50
#define SST25VF_INS_RD_STATUS 0x05
#define SST25VF_INS_WR_STATUS 0x01
#define SST25VF_INS_RD_ID 0x9F
#define SST25VF_INS_POWER_DOWN 0xB9
#define SST25VF_INS_POWER_UP 0xAB
#define SST25VF_INS_RD_ID 0x9F
#define SST25VF_INS_EBSY 0x70
#define SST25VF_INS_DBSY 0x80

#define SST25VF_STATUS_BUSY_BIT 0
#define SST25VF_STATUS_WEL_BIT 1
#define SST25VF_STATUS_SWPL_BIT 7

#define SST25VF_STATUS_SWP_BIT0 2
#define SST25VF_STATUS_SWP_BIT1 3
#define SST25VF_STATUS_SWP_BIT2 4
#define SST25VF_STATUS_SWP_BIT3 5

void sst25vf_share_assert_cs(const devfs_handle_t * handle);
void sst25vf_share_deassert_cs(const devfs_handle_t * handle);
void sst25vf_share_write_enable(const devfs_handle_t * handle);
void sst25vf_share_write_disable(const devfs_handle_t * handle);
void sst25vf_share_write_ebsy(const devfs_handle_t * handle);
void sst25vf_share_write_dbsy(const devfs_handle_t * handle);
void sst25vf_share_write_opcode_addr(const devfs_handle_t * handle, uint8_t opcode, uint32_t addr);
void sst25vf_share_write_quick_opcode(const devfs_handle_t * handle, uint8_t opcode);
void sst25vf_share_block_erase_4kb(const devfs_handle_t * handle, uint32_t addr);
void sst25vf_share_block_erase_32kb(const devfs_handle_t * handle, uint32_t addr);
void sst25vf_share_block_erase_64kb(const devfs_handle_t * handle, uint32_t addr);
void sst25vf_share_chip_erase(const devfs_handle_t * handle);
void sst25vf_share_write_byte(const devfs_handle_t * handle, uint32_t addr, char byte);
char sst25vf_share_read_byte(const devfs_handle_t * handle, uint32_t addr);
uint8_t sst25vf_share_read_status(const devfs_handle_t * handle);
void sst25vf_share_write_status(const devfs_handle_t * handle, uint8_t status);
void sst25vf_share_power_down(const devfs_handle_t * handle);
void sst25vf_share_power_up(const devfs_handle_t * handle);
void sst25vf_share_global_protect(const devfs_handle_t * handle);
int sst25vf_share_global_unprotect(const devfs_handle_t * handle);
void sst25vf_share_read_id(const devfs_handle_t * handle, char * dest);
int sst25vf_share_ioctl(const devfs_handle_t * handle, int request, void * ctl);

#endif /* SST25VF_LOCAL_H_ */
