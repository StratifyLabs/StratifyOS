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


#include <errno.h>
#include <unistd.h>
#include "mcu/mcu.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/debug.h"

#include "sst25vf_local.h"


void sst25vf_share_assert_cs(const devfs_handle_t * cfg){
	const sst25vf_cfg_t * config = cfg->config;
	mcu_pio_clrmask(config->cs.port, (void*)(ssize_t)(1<<config->cs.pin));
}

void sst25vf_share_deassert_cs(const devfs_handle_t * cfg){
	const sst25vf_cfg_t * config = cfg->config;
	mcu_pio_setmask(config->cs.port, (void*)(ssize_t)(1<<config->cs.pin));
}


void sst25vf_share_write_enable(const devfs_handle_t * cfg){
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_WRITE_ENABLE);
}

void sst25vf_share_write_disable(const devfs_handle_t * cfg){
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_WRITE_DISABLE);
}

void sst25vf_share_write_ebsy(const devfs_handle_t * cfg){
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_EBSY);
}

void sst25vf_share_write_dbsy(const devfs_handle_t * cfg){
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_DBSY);
}

void sst25vf_share_write_opcode_addr(const devfs_handle_t * cfg, uint8_t opcode, uint32_t addr){
	uint8_t * addrp;
	addrp = (uint8_t*)&addr;
	//send the opcode
	mcu_spi_swap(cfg->port, (void*)(ssize_t)opcode);
	//send the 3-byte address MSB to LSB (assumes little-endian arch)
	mcu_spi_swap(cfg->port, (void*)(ssize_t)addrp[2]);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)addrp[1]);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)addrp[0]);
}

void sst25vf_share_write_quick_opcode(const devfs_handle_t * cfg, uint8_t opcode){
	sst25vf_share_assert_cs(cfg);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)opcode);
	sst25vf_share_deassert_cs(cfg);
}

void sst25vf_share_block_erase_4kb(const devfs_handle_t * cfg, uint32_t addr /*! Any address in the 4KB block */){
	sst25vf_share_write_enable(cfg);
	sst25vf_share_assert_cs(cfg);
	sst25vf_share_write_opcode_addr(cfg, SST25VF_INS_BLOCK_ERASE_4KB, addr);
	sst25vf_share_deassert_cs(cfg);
}

void sst25vf_share_block_erase_32kb(const devfs_handle_t * cfg, uint32_t addr /*! Any address in the 32KB block */){
	sst25vf_share_write_enable(cfg);
	sst25vf_share_assert_cs(cfg);
	sst25vf_share_write_opcode_addr(cfg, SST25VF_INS_BLOCK_ERASE_32KB, addr);
	sst25vf_share_deassert_cs(cfg);
}

void sst25vf_share_block_erase_64kb(const devfs_handle_t * cfg, uint32_t addr /*! Any address in the 64KB block */){
	sst25vf_share_write_enable(cfg);
	sst25vf_share_assert_cs(cfg);
	sst25vf_share_write_opcode_addr(cfg, SST25VF_INS_BLOCK_ERASE_64KB, addr);
	sst25vf_share_deassert_cs(cfg);
}

void sst25vf_share_chip_erase(const devfs_handle_t * cfg){
	sst25vf_share_write_enable(cfg);
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_CHIP_ERASE);
	sst25vf_share_read_status(cfg);
}

void sst25vf_share_write_byte(const devfs_handle_t * cfg, uint32_t addr, char byte){
	sst25vf_share_write_enable(cfg);
	sst25vf_share_assert_cs(cfg);
	sst25vf_share_write_opcode_addr(cfg, SST25VF_INS_PROGRAM, addr);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)byte);
	sst25vf_share_deassert_cs(cfg);
}

char sst25vf_share_read_byte(const devfs_handle_t * cfg, uint32_t addr){
	char byte;
	sst25vf_share_assert_cs(cfg);
	sst25vf_share_write_opcode_addr(cfg, SST25VF_INS_RD_HS, addr);
	mcu_spi_swap(cfg->port, 0); //dummy byte output
	byte = mcu_spi_swap(cfg->port, (void*)(ssize_t)0xFF);
	sst25vf_share_deassert_cs(cfg);
	return byte;
}

uint8_t sst25vf_share_read_status(const devfs_handle_t * cfg){
	uint8_t status;
	sst25vf_share_assert_cs(cfg);
	mcu_spi_swap(cfg->port, (void*)SST25VF_INS_RD_STATUS);
	status = mcu_spi_swap(cfg->port, (void*)(ssize_t)0xFF);
	sst25vf_share_deassert_cs(cfg);
	return status;
}

void sst25vf_share_write_status(const devfs_handle_t * cfg, uint8_t status){
	sst25vf_share_write_enable(cfg);
	sst25vf_share_assert_cs(cfg);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)SST25VF_INS_WR_STATUS);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)status);
	sst25vf_share_deassert_cs(cfg);
}

void sst25vf_share_power_down(const devfs_handle_t * cfg){
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_POWER_DOWN);
}

void sst25vf_share_power_up(const devfs_handle_t * cfg){
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_POWER_UP);
}

void sst25vf_share_global_protect(const devfs_handle_t * cfg){
	//maninpulate the status registers
	uint8_t status;
	status = (1<<SST25VF_STATUS_SWPL_BIT) |
			(1<<SST25VF_STATUS_SWP_BIT0) |
			(1<<SST25VF_STATUS_SWP_BIT1) |
			(1<<SST25VF_STATUS_SWP_BIT2) |
			(1<<SST25VF_STATUS_SWP_BIT3);
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_WR_STATUS_ENABLE);
	sst25vf_share_write_status(cfg, status); //global protect and set SWPL
}

int sst25vf_share_global_unprotect(const devfs_handle_t * cfg){
	//manipulate the status registers
	sst25vf_share_write_quick_opcode(cfg, SST25VF_INS_WR_STATUS_ENABLE);
	sst25vf_share_write_status(cfg, (1<<SST25VF_STATUS_SWPL_BIT)); //global unprotect and set SPRL
	return 0;
}

void sst25vf_share_read_id(const devfs_handle_t * cfg, char * dest){
	int i;
	sst25vf_share_assert_cs(cfg);
	mcu_spi_swap(cfg->port, (void*)(ssize_t)SST25VF_INS_RD_ID);
	for(i=0; i < 4; i++){
		dest[i] = mcu_spi_swap(cfg->port, (void*)(ssize_t)0xFF);
	}
	sst25vf_share_deassert_cs(cfg);
}




/*! @} */

