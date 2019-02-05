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

#include "mcu/spi.h"

#define mcu_spi_open mcu_spi_dma_open
#define mcu_spi_close mcu_spi_dma_close
#define mcu_spi_setaction mcu_spi_dma_setaction
#define mcu_spi_setduplex mcu_spi_dma_setduplex
#define mcu_spi_swap mcu_spi_dma_swap
#define mcu_spi_getinfo mcu_spi_dma_getinfo
#define mcu_spi_setattr mcu_spi_dma_setattr
#define mcu_spi_write mcu_spi_dma_write
#define mcu_spi_read mcu_spi_dma_read
#define mcu_spi_ioctl mcu_spi_dma_ioctl


#define drive_sdspi_open drive_sdspi_dma_open
#define drive_sdspi_ioctl drive_sdspi_dma_ioctl
#define drive_sdspi_read drive_sdspi_dma_read
#define drive_sdspi_write drive_sdspi_dma_write
#define drive_sdspi_close drive_sdspi_dma_close


#include "drive_sdspi.c"


