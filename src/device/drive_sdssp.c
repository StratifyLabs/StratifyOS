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


#define drive_sdspi_open drive_sdssp_open
#define drive_sdspi_ioctl drive_sdssp_ioctl
#define drive_sdspi_read drive_sdssp_read
#define drive_sdspi_write drive_sdssp_write
#define drive_sdspi_close drive_sdssp_close


#include "drive_sdspi.c"


