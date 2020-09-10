/* Copyright 2016 Tyler Gilbert;
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

#include "mcu/spi.h"

#define mcu_spi_open mcu_ssp_open
#define mcu_spi_close mcu_ssp_close
#define mcu_spi_setaction mcu_ssp_setaction
#define mcu_spi_setduplex mcu_ssp_setduplex
#define mcu_spi_swap mcu_ssp_swap
#define mcu_spi_getinfo mcu_ssp_getinfo
#define mcu_spi_setattr mcu_ssp_setattr
#define mcu_spi_write mcu_ssp_write
#define mcu_spi_read mcu_ssp_read
#define mcu_spi_ioctl mcu_ssp_ioctl

#define sst25vf_tmr_open sst25vf_ssp_tmr_open
#define sst25vf_tmr_ioctl sst25vf_ssp_tmr_ioctl
#define sst25vf_tmr_read sst25vf_ssp_tmr_read
#define sst25vf_tmr_write sst25vf_ssp_tmr_write
#define sst25vf_tmr_close sst25vf_ssp_tmr_close

#define sst25vf_share_assert_cs sst25vf_ssp_tmr_local_assert_cs
#define sst25vf_share_deassert_cs sst25vf_ssp_tmr_local_deassert_cs
#define sst25vf_share_write_enable sst25vf_ssp_tmr_local_write_enable
#define sst25vf_share_write_disable sst25vf_ssp_tmr_local_write_disable
#define sst25vf_share_write_ebsy sst25vf_ssp_tmr_local_write_ebsy
#define sst25vf_share_write_dbsy sst25vf_ssp_tmr_local_write_dbsy
#define sst25vf_share_write_opcode_addr sst25vf_ssp_tmr_local_write_opcode_addr
#define sst25vf_share_write_quick_opcode sst25vf_ssp_tmr_local_write_quick_opcode
#define sst25vf_share_block_erase_4kb sst25vf_ssp_tmr_local_block_erase_4kb
#define sst25vf_share_block_erase_32kb sst25vf_ssp_tmr_local_block_erase_32kb
#define sst25vf_share_block_erase_64kb sst25vf_ssp_tmr_local_block_erase_64kb
#define sst25vf_share_chip_erase sst25vf_ssp_tmr_local_chip_erase
#define sst25vf_share_write_byte sst25vf_ssp_tmr_local_write_byte
#define sst25vf_share_read_byte sst25vf_ssp_tmr_local_read_byte
#define sst25vf_share_read_status sst25vf_ssp_tmr_local_read_status
#define sst25vf_share_write_status sst25vf_ssp_tmr_local_write_status
#define sst25vf_share_power_down sst25vf_ssp_tmr_local_power_down
#define sst25vf_share_power_up sst25vf_ssp_tmr_local_power_up
#define sst25vf_share_global_protect sst25vf_ssp_tmr_local_global_protect
#define sst25vf_share_global_unprotect sst25vf_ssp_tmr_local_global_unprotect
#define sst25vf_share_read_id sst25vf_ssp_tmr_local_read_id


#include "sst25vf_tmr.c"
#include "sst25vf_share.c"
