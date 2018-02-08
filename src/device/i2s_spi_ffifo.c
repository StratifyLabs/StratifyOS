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

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "device/usbfifo.h"
#include "mcu/usb.h"
#include "mcu/debug.h"

#define i2s_ffifo_open i2s_spi_ffifo_open
#define i2s_ffifo_close i2s_spi_ffifo_close
#define i2s_ffifo_read i2s_spi_ffifo_read
#define i2s_ffifo_write i2s_spi_ffifo_write
#define i2s_ffifo_ioctl i2s_spi_ffifo_ioctl

#define mcu_i2s_open mcu_i2s_spi_open
#define mcu_i2s_close mcu_i2s_spi_close
#define mcu_i2s_read mcu_i2s_spi_read
#define mcu_i2s_write mcu_i2s_spi_write
#define mcu_i2s_ioctl mcu_i2s_spi_ioctl


#include "i2s_ffifo.c"
