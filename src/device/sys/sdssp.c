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


#define sdspi_open sdssp_open
#define sdspi_ioctl sdssp_ioctl
#define sdspi_read sdssp_read
#define sdspi_write sdssp_write
#define sdspi_close sdssp_close


#include "sdspi.c"


