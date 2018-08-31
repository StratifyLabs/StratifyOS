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

#include "mcu/mmc.h"

#define mcu_mmc_open mcu_mmc_dma_open
#define mcu_mmc_close mcu_mmc_dma_close
#define mcu_mmc_setaction mcu_mmc_dma_setaction
#define mcu_mmc_getinfo mcu_mmc_dma_getinfo
#define mcu_mmc_setattr mcu_mmc_dma_setattr
#define mcu_mmc_write mcu_mmc_dma_write
#define mcu_mmc_read mcu_mmc_dma_read
#define mcu_mmc_ioctl mcu_mmc_dma_ioctl
#define mcu_mmc_getcsd mcu_mmc_dma_getcsd
#define mcu_mmc_getcid mcu_mmc_dma_getcid
#define mcu_mmc_getstatus mcu_mmc_dma_getstatus


#define drive_mmc_open drive_mmc_dma_open
#define drive_mmc_ioctl drive_mmc_dma_ioctl
#define drive_mmc_read drive_mmc_dma_read
#define drive_mmc_write drive_mmc_dma_write
#define drive_mmc_close drive_mmc_dma_close


#include "drive_sdio.c"


