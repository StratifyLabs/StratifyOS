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

#ifndef DEVICE_CFI_H_
#define DEVICE_CFI_H_

#include "sos/fs/devfs.h"
#include "sos/dev/drive.h"
#include "mcu/spi.h"
#include "mcu/qspi.h"

//Serial Flash Discoverable Parameters -- the minimum need for the driver to work
typedef struct {
    u32 o_flags;
    u8 opcode_4kb_erase;
    u8 opcode_write_enable_status;
    u32 size;
    u8 opcode_fast_read_114;
    u8 opcode_fast_read_114_mode_bits;
    u8 opcode_fast_read_114_wait_states;
    u8 opcode_fast_read_144;
    u8 opcode_fast_read_144_mode_bits;
    u8 opcode_fast_read_144_wait_states;
    u8 opcode_fast_read_122;
    u8 opcode_fast_read_122_mode_bits;
    u8 opcode_fast_read_122_wait_states;
    u8 opcode_fast_read_112;
    u8 opcode_fast_read_112_mode_bits;
    u8 opcode_fast_read_112_wait_states;
    u8 opcode_fast_read_222;
    u8 opcode_fast_read_222_mode_bits;
    u8 opcode_fast_read_222_wait_states;
    u8 opcode_fast_read_444;
    u8 opcode_fast_read_444_mode_bits;
    u8 opcode_fast_read_444_wait_states;
    u8 opcode_erase_size2;
    u8 erase_size2;
    u8 opcode_erase_size1;
    u8 erase_size1;
    u8 opcode_erase_size4;
    u8 erase_size4;
    u8 opcode_erase_size3;
    u8 erase_size3;
} drive_cfi_sfdp_t;


typedef struct {
    mcu_event_handler_t handler;
    drive_cfi_sfdp_t sfdp;
} drive_cfi_state_t;

typedef struct {
    mcu_pin_t cs;
    const void * mcu_config;
} drive_cfi_config_t;


DEVFS_DRIVER_DECLARTION(cfi_spi);
DEVFS_DRIVER_DECLARTION(cfi_spi_dma);
DEVFS_DRIVER_DECLARTION(cfi_qspi);
DEVFS_DRIVER_DECLARTION(cfi_qspi_dma);



#endif /* DEVICE_CFI_H_ */
