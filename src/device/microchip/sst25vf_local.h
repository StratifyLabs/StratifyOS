/*
 * sst25vf_local.h
 *
 *  Created on: Dec 26, 2012
 *      Author: tgil
 */

#ifndef SST25VF_LOCAL_H_
#define SST25VF_LOCAL_H_

#include <errno.h>
#include <unistd.h>
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/debug.h"
#include "iface/dev/microchip/sst25vf.h"
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

void sst25vf_share_assert_cs(const device_cfg_t * cfg);
void sst25vf_share_deassert_cs(const device_cfg_t * cfg);
void sst25vf_share_write_enable(const device_cfg_t * cfg);
void sst25vf_share_write_disable(const device_cfg_t * cfg);
void sst25vf_share_write_ebsy(const device_cfg_t * cfg);
void sst25vf_share_write_dbsy(const device_cfg_t * cfg);
void sst25vf_share_write_opcode_addr(const device_cfg_t * cfg, uint8_t opcode, uint32_t addr);
void sst25vf_share_write_quick_opcode(const device_cfg_t * cfg, uint8_t opcode);
void sst25vf_share_block_erase_4kb(const device_cfg_t * cfg, uint32_t addr);
void sst25vf_share_block_erase_32kb(const device_cfg_t * cfg, uint32_t addr);
void sst25vf_share_block_erase_64kb(const device_cfg_t * cfg, uint32_t addr);
void sst25vf_share_chip_erase(const device_cfg_t * cfg);
void sst25vf_share_write_byte(const device_cfg_t * cfg, uint32_t addr, char byte);
char sst25vf_share_read_byte(const device_cfg_t * cfg, uint32_t addr);
uint8_t sst25vf_share_read_status(const device_cfg_t * cfg);
void sst25vf_share_write_status(const device_cfg_t * cfg, uint8_t status);
void sst25vf_share_power_down(const device_cfg_t * cfg);
void sst25vf_share_power_up(const device_cfg_t * cfg);
void sst25vf_share_global_protect(const device_cfg_t * cfg);
int sst25vf_share_global_unprotect(const device_cfg_t * cfg);
void sst25vf_share_read_id(const device_cfg_t * cfg, char * dest);


#endif /* SST25VF_LOCAL_H_ */
