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

#include "mcu/sdio.h"

#define mcu_sdio_open mcu_sdio_dma_open
#define mcu_sdio_close mcu_sdio_dma_close
#define mcu_sdio_setaction mcu_sdio_dma_setaction
#define mcu_sdio_getinfo mcu_sdio_dma_getinfo
#define mcu_sdio_setattr mcu_sdio_dma_setattr
#define mcu_sdio_write mcu_sdio_dma_write
#define mcu_sdio_read mcu_sdio_dma_read
#define mcu_sdio_ioctl mcu_sdio_dma_ioctl
#define mcu_sdio_getcsd mcu_sdio_dma_getcsd
#define mcu_sdio_getcid mcu_sdio_dma_getcid
#define mcu_sdio_getstatus mcu_sdio_dma_getstatus


#define drive_sdio_open drive_sdio_dma_open
#define drive_sdio_ioctl drive_sdio_dma_ioctl
#define drive_sdio_read drive_sdio_dma_read
#define drive_sdio_write drive_sdio_dma_write
#define drive_sdio_close drive_sdio_dma_close


#include "drive_sdio.c"


