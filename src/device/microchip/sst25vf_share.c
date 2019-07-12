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


#include <errno.h>
#include <unistd.h>
#include "mcu/mcu.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/debug.h"

#include "sst25vf_local.h"


void sst25vf_share_assert_cs(const devfs_handle_t * handle){
	 devfs_handle_t pio_handle;
	 const sst25vf_config_t * config = handle->config;
	 pio_handle.port = config->cs.port;
	 pio_handle.config = 0;
	 mcu_pio_clrmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

void sst25vf_share_deassert_cs(const devfs_handle_t * handle){
	 devfs_handle_t pio_handle;
	 const sst25vf_config_t * config = handle->config;
	 pio_handle.port = config->cs.port;
	 pio_handle.config = 0;
	 mcu_pio_setmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}


void sst25vf_share_write_enable(const devfs_handle_t * handle){
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_WRITE_ENABLE);
}

void sst25vf_share_write_disable(const devfs_handle_t * handle){
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_WRITE_DISABLE);
}

void sst25vf_share_write_ebsy(const devfs_handle_t * handle){
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_EBSY);
}

void sst25vf_share_write_dbsy(const devfs_handle_t * handle){
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_DBSY);
}

void sst25vf_share_write_opcode_addr(const devfs_handle_t * handle, uint8_t opcode, uint32_t addr){
	 uint8_t * addrp;
	 addrp = (uint8_t*)&addr;
	 //send the opcode
	 mcu_spi_swap(handle, (void*)(ssize_t)opcode);
	 //send the 3-byte address MSB to LSB (assumes little-endian arch)
	 mcu_spi_swap(handle, (void*)(ssize_t)addrp[2]);
	 mcu_spi_swap(handle, (void*)(ssize_t)addrp[1]);
	 mcu_spi_swap(handle, (void*)(ssize_t)addrp[0]);
}

void sst25vf_share_write_quick_opcode(const devfs_handle_t * handle, uint8_t opcode){
	 sst25vf_share_assert_cs(handle);
	 mcu_spi_swap(handle, (void*)(ssize_t)opcode);
	 sst25vf_share_deassert_cs(handle);
}

void sst25vf_share_block_erase_4kb(const devfs_handle_t * handle, uint32_t addr /*! Any address in the 4KB block */){
	 sst25vf_share_write_enable(handle);
	 sst25vf_share_assert_cs(handle);
	 sst25vf_share_write_opcode_addr(handle, SST25VF_INS_BLOCK_ERASE_4KB, addr);
	 sst25vf_share_deassert_cs(handle);
}

void sst25vf_share_block_erase_32kb(const devfs_handle_t * handle, uint32_t addr /*! Any address in the 32KB block */){
	 sst25vf_share_write_enable(handle);
	 sst25vf_share_assert_cs(handle);
	 sst25vf_share_write_opcode_addr(handle, SST25VF_INS_BLOCK_ERASE_32KB, addr);
	 sst25vf_share_deassert_cs(handle);
}

void sst25vf_share_block_erase_64kb(const devfs_handle_t * handle, uint32_t addr /*! Any address in the 64KB block */){
	 sst25vf_share_write_enable(handle);
	 sst25vf_share_assert_cs(handle);
	 sst25vf_share_write_opcode_addr(handle, SST25VF_INS_BLOCK_ERASE_64KB, addr);
	 sst25vf_share_deassert_cs(handle);
}

void sst25vf_share_chip_erase(const devfs_handle_t * handle){
	 sst25vf_share_write_enable(handle);
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_CHIP_ERASE);
	 sst25vf_share_read_status(handle);
}

void sst25vf_share_write_byte(const devfs_handle_t * handle, uint32_t addr, char byte){
	 sst25vf_share_write_enable(handle);
	 sst25vf_share_assert_cs(handle);
	 sst25vf_share_write_opcode_addr(handle, SST25VF_INS_PROGRAM, addr);
	 mcu_spi_swap(handle, (void*)(ssize_t)byte);
	 sst25vf_share_deassert_cs(handle);
}

char sst25vf_share_read_byte(const devfs_handle_t * handle, uint32_t addr){
	 char byte;
	 sst25vf_share_assert_cs(handle);
	 sst25vf_share_write_opcode_addr(handle, SST25VF_INS_RD_HS, addr);
	 mcu_spi_swap(handle, 0); //dummy byte output
	 byte = mcu_spi_swap(handle, (void*)(ssize_t)0xFF);
	 sst25vf_share_deassert_cs(handle);
	 return byte;
}

uint8_t sst25vf_share_read_status(const devfs_handle_t * handle){
	 uint8_t status;
	 sst25vf_share_assert_cs(handle);
	 mcu_spi_swap(handle, (void*)SST25VF_INS_RD_STATUS);
	 status = mcu_spi_swap(handle, (void*)(ssize_t)0xFF);
	 sst25vf_share_deassert_cs(handle);
	 return status;
}

void sst25vf_share_write_status(const devfs_handle_t * handle, uint8_t status){
	 sst25vf_share_write_enable(handle);
	 sst25vf_share_assert_cs(handle);
	 mcu_spi_swap(handle, (void*)(ssize_t)SST25VF_INS_WR_STATUS);
	 mcu_spi_swap(handle, (void*)(ssize_t)status);
	 sst25vf_share_deassert_cs(handle);
}

void sst25vf_share_power_down(const devfs_handle_t * handle){
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_POWER_DOWN);
}

void sst25vf_share_power_up(const devfs_handle_t * handle){
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_POWER_UP);
}

void sst25vf_share_global_protect(const devfs_handle_t * handle){
	 //maninpulate the status registers
	 uint8_t status;
	 status = (1<<SST25VF_STATUS_SWPL_BIT) |
				(1<<SST25VF_STATUS_SWP_BIT0) |
				(1<<SST25VF_STATUS_SWP_BIT1) |
				(1<<SST25VF_STATUS_SWP_BIT2) |
				(1<<SST25VF_STATUS_SWP_BIT3);
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_WR_STATUS_ENABLE);
	 sst25vf_share_write_status(handle, status); //global protect and set SWPL
}

int sst25vf_share_global_unprotect(const devfs_handle_t * handle){
	 //manipulate the status registers
	 sst25vf_share_write_quick_opcode(handle, SST25VF_INS_WR_STATUS_ENABLE);
	 sst25vf_share_write_status(handle, (1<<SST25VF_STATUS_SWPL_BIT)); //global unprotect and set SPRL
	 return 0;
}

void sst25vf_share_read_id(const devfs_handle_t * handle, char * dest){
	 int i;
	 sst25vf_share_assert_cs(handle);
	 mcu_spi_swap(handle, (void*)(ssize_t)SST25VF_INS_RD_ID);
	 for(i=0; i < 4; i++){
		  dest[i] = mcu_spi_swap(handle, (void*)(ssize_t)0xFF);
	 }
	 sst25vf_share_deassert_cs(handle);
}

int sst25vf_share_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	 sst25vf_config_t * sst_cfg = (sst25vf_config_t*)handle->config;
	 sst25vf_state_t * state = (sst25vf_state_t*)handle->state;
	 drive_info_t * info;
	 drive_attr_t * attr;
	 u32 o_flags;
	 int i;

	 switch(request){
	 case I_DRIVE_GETVERSION:
		  return DRIVE_VERSION;

	 case I_DRIVE_SETATTR:
		  attr = ctl;
		  o_flags = attr->o_flags;

		  if( o_flags & (DRIVE_FLAG_ERASE_DEVICE|DRIVE_FLAG_ERASE_BLOCKS) ){
				if( state->prot == 1 ){
					 return SYSFS_SET_RETURN(EROFS);
				}

				if( o_flags & DRIVE_FLAG_ERASE_DEVICE ){
					 sst25vf_share_chip_erase(handle);
				}

				if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){
					 for(i=attr->start; i <= attr->end; i++){
						  sst25vf_share_block_erase_4kb(handle, attr->start);
					 }
				}
		  }

		  if( o_flags & DRIVE_FLAG_PROTECT ){
				sst25vf_share_global_protect(handle);
		  }

		  if( o_flags & DRIVE_FLAG_UNPROTECT ){
				sst25vf_share_global_unprotect(handle);
		  }

		  if( o_flags & DRIVE_FLAG_POWERUP ){
				sst25vf_share_power_up(handle);
		  }

		  if( o_flags & DRIVE_FLAG_POWERDOWN ){
				sst25vf_share_power_down(handle);
		  }

		  break;
	 case I_DRIVE_GETINFO:
		  info = ctl;
		  info->addressable_size = 1;
		  info->bitrate = 50000000;
		  info->erase_block_size = SST25VF_BLOCK_ERASE_SIZE;
		  info->erase_block_time = SST25VF_BLOCK_ERASE_TIME;
		  info->erase_device_time = SST25VF_CHIP_ERASE_TIME;
		  info->num_write_blocks = sst_cfg->size / SST25VF_BLOCK_SIZE;
		  info->write_block_size = SST25VF_BLOCK_SIZE;
		  return 0;
	 default:
		  return mcu_spi_ioctl(handle, request, ctl);
	 }

	 return 0;
}




/*! @} */

